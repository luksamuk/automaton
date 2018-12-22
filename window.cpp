#include "macros.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <cstring>

/* ========================================================================== */
/*                              Variáveis externas                            */
/* ========================================================================== */

// As variáveis e funções a seguir foram declaradas em `main.cpp`.

// Provê acesso externo às duas matrizes do autômato.
extern int cur_grid[AUTOMATON_HEIGHT][AUTOMATON_WIDTH];
extern int old_grid[AUTOMATON_HEIGHT][AUTOMATON_WIDTH];

// Provê acesso a algumas funções básicas para iterar o autômato.
extern void initialize_automata();
extern void copy_last_state();
extern void apply_rules();

/* ========================================================================== */
/*                              Macros e Estruturas                           */
/* ========================================================================== */

/* Tamanho das células a serem renderizadas na tela */
#define MIN(x, y)   (x < y ? x : y)
#define CELL_WIDTH  (2.0 / AUTOMATON_WIDTH)
#define CELL_HEIGHT (2.0 / AUTOMATON_HEIGHT)
#define CELL_SIZE   (MIN(CELL_WIDTH, CELL_HEIGHT))

/* Valores relacionados ao input do usuário */
struct user_input_t {
    int  cursor_grid_x;
    int  cursor_grid_y;
    bool excite_cell;
    bool cleanup;
    bool paused;
};

/* Valores relacionados a instâncias da janela */
struct window_info_t {
    GLFWwindow* ptr;
    double      width;
    double      height;
    double      last_swap;
    double      refresh_interval;
};


/* Instâncias das estruturas acima, inacessíveis em outros arquivos */
static user_input_t  input  = {};
static window_info_t window = { NULL, 640.0, 640.0, 0.0, 0.025 };


/* ========================================================================== */
/*                               Protótipos                                  */
/* ========================================================================== */


/* Protótipos de callbacks a serem utilizados na criação da janela.
 * As definições encontram-se ao fim do arquivo. */
static void resize_window_callback(GLFWwindow*, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void keyboard_callback(GLFWwindow*, int, int, int, int);

/* Protótipos de funções de renderização.
 * As definições encontram-se também ao fim do arquivo. */
static void render_grid_lines();
static void render_grid_cell(int, int, int);
static void render_grid();


/* ========================================================================== */
/*                           Funções essenciais                               */
/* ========================================================================== */

// Tenta criar a janela. O resultado da operação é indicado no retorno.
bool
create_window()
{
    // Tentativa de inicializar o GLFW
    if(!glfwInit()) {
        return false;
    }

    /* Hints para configuração inicial da janela pelo GLFW.
     * Aqui usamos OpenGL 2.1 para ter acesso à pipeline fixa. Para
     * uma aplicação simples como esta, não convém utilizar shaders
     * ou bibliotecas de matrizes. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Criação da janela
    window.ptr =
        glfwCreateWindow(window.width, window.height,
                         "Trabalho Prático de AEDS I",
                         NULL, NULL);

    // Não faz sentido continuar se a janela não foi criada
    if(!window.ptr) {
        glfwTerminate();
        return false;
    }

    /* Definição de callbacks para a janela. Com estas definições,
     * algumas das entradas fornecidas através de eventos na janela serão
     * despachadas para estas funções. */
    glfwSetWindowSizeCallback(window.ptr, resize_window_callback);
    glfwSetCursorPosCallback(window.ptr, cursor_pos_callback);
    glfwSetMouseButtonCallback(window.ptr, mouse_button_callback);
    glfwSetKeyCallback(window.ptr, keyboard_callback);

    /* As definições a seguir dizem respeito à inicialização do OpenGL */

    // Força a utilização do contexto OpenGL na thread principal do programa
    glfwMakeContextCurrent(window.ptr);

    // Altera a cor de fundo do contexto
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Redefine a posição inicial da matriz de projeção. Equivale a uma chamada
    // do callback de redimensionamento, então... chamamos ele logo.
    resize_window_callback(window.ptr, (int) window.width, (int) window.height);
    
    return true;
}

// Dispõe da janela, após a utilização da mesma. Esta função não é exportada.
static void
dispose_window()
{
    if(window.ptr) {
        // A janela será encerrada, assim como o GLFW.
        glfwDestroyWindow(window.ptr);
        glfwTerminate();
    }
}

// Lida com os eventos de entrada produzidos pelo usuário na janela.
// Esta função não é exportada.
static void
handle_events()
{
    // Excita a célula sob a qual o cursor está, se o usuário clicou nela.
    if(input.excite_cell) {
        cur_grid[input.cursor_grid_y][input.cursor_grid_x] = CELL_EXCITED;
        input.excite_cell = false;
    }

    // Limpa a tela, se o usuário apertou o botão no teclado.
    if(input.cleanup) {
        initialize_automata(); // Função importada direto do autômato
        input.cleanup = false;
    }
}

// Efetivamente itera o autômato, nos passos de tempo apropriados para
// renderização na tela. Esta função não é exportada.
static void
automata_gui_update()
{
    /* Controle de iterações por segundo */
        
    /* Garante um máximo de uma iteração de 1 quadro renderizado a cada
     * 1/75 segundo, efetivamente limitando a quantidade aparente de quadros
     * renderizados.
     *
     * O motivo para isto é que, ao invés de limitarmos quantas vezes este
     * loop itera por segundo, devemos limitar as iterações da lógica do
     * autômato em si. Dessa forma, garantimos que a janela responda
     * imediatamente a entradas do usuário. */
    
    if(!input.paused) {
        double current_time = glfwGetTime();

        if(current_time - window.last_swap >= window.refresh_interval) {
            window.last_swap = current_time;

            // Aplica as regras no autômato
            copy_last_state();
            apply_rules();
        }
    }
}


// Realiza os ciclos das gerações do autômato, renderizando-o na janela.
// Esta função é utilizada na renderização gráfica.
void
automata_gui_loop()
{
    // O loop para a interface gráfica acontece continuamente, se e somente se
    // a janela não tiver recebido um evento de encerramento.
    while(!glfwWindowShouldClose(window.ptr)) {
        // Despachamos os eventos para seus devidos callbacks
        glfwPollEvents();

        // Lidamos com os eventos de entrada fornecidos pelo usuário
        handle_events();

        // Atualizamos o autômato, a rigor
        automata_gui_update();

        /* Renderização do autômato */
        // Limpa a tela
        glClear(GL_COLOR_BUFFER_BIT);
        // Renderização
        render_grid();
        // Swap no buffer de renderização, para que seja mostrado na tela.
        glfwSwapBuffers(window.ptr);
    }

    // Ao final, dispomos a janela e o GLFW.
    dispose_window();
}




/* ========================================================================== */
/*                         Callbacks para a Janela                            */
/* ========================================================================== */

/* Callbacks são funções que serão chamadas mediante eventos específicos da
 * janela, desde que pré-configuradas para tal. Em outra situação, seria
 * interessante definir diretamente os callbacks como expressões lambda
 * diretamente em seu local de uso, mas aqui serão deixadas de forma explícita.
 *
 * Todas estas funções são estáticas, portanto não podem ser acessadas fora
 * deste arquivo.
 */

/* Callback de redimensionamento de janela. Escalona o viewport para a janela */
static void
resize_window_callback(GLFWwindow* ptr, int x, int y)
{
    glViewport(0, 0, x, y);
    window.width  = (double) x;
    window.height = (double) y;
}

/* Callback de posicionamento do cursor. Fornece a posição do cursor a cada
 * movimento do mouse */
static void
cursor_pos_callback(GLFWwindow* ptr, double x, double y)
{
    /* GLFW cede a posição do mouse relativa ao canto superior esquerdo da
     * janela, mas OpenGL renderiza usando um plano cartesiano, onde a origem
     * está no exato centro da tela, e as extremidades dos eixos são unitárias
     * (ex: {-1, 1} via esquerda->direita; {-1, 1} via baixo->topo). Corrigimos
     * isto matematicamente e atribuimos à posição global armazenada do cursor
     * os índices exatos da célula à qual o cursor se sobrepõe. */
    
    x /= window.width  * CELL_SIZE / 2.0;
    y /= window.height * CELL_SIZE / 2.0;

    input.cursor_grid_x = (int) x;
    input.cursor_grid_y = (int) y;
}

/* Callback de pressionamento de botões do mouse. */
static void
mouse_button_callback(GLFWwindow* ptr, int button, int action, int mod)
{
    switch(button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        // Botão esquerdo excita a célula sobre a qual o cursor está
        if(action == GLFW_PRESS) {
            input.excite_cell = true;
        }
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        // Botão direito pausa/despausa a execução das regras do autômato
        if(action == GLFW_PRESS) {
            input.paused = !input.paused;
        }
        break;
    default: break;
    }
}

/* Callback de pressionamento de botões do teclado. */
static void
keyboard_callback(GLFWwindow* ptr, int key, int scancod, int action, int mod)
{
    switch(key) {
    case GLFW_KEY_C:
        // Pressionar 'c' limpa o autômato
        if(action == GLFW_PRESS) {
            input.cleanup = true;
        }
        break;
    // Aumentando e diminuindo a velocidade de evolução do autômato
    case GLFW_KEY_MINUS:
        if(action == GLFW_PRESS) {
            window.refresh_interval += 0.025;
        }
        break;
    case GLFW_KEY_EQUAL:
        if(action == GLFW_PRESS) {
            window.refresh_interval -= 0.025;
            window.refresh_interval =
                (window.refresh_interval < 0.0 ? 0.0 : window.refresh_interval);
        }
        break;
    default: break;
    }
}


/* ========================================================================== */
/*                         Funções para renderização                          */
/* ========================================================================== */

// Renderiza as linhas da grade, como mostradas durante a pausa da aplicação.
static void
render_grid_lines()
{
    // As linhas têm cor esverdeada
    glColor3f(0.2f, 0.6f, 0.3f);

    // Linhas horizontais
    for(int i = 0; i < AUTOMATON_WIDTH; i++) {
        float cell_x = -1.0f + (i * CELL_SIZE);
        glBegin(GL_LINES);
        glVertex2f(cell_x, 1.0f);
        glVertex2f(cell_x, -1.0f);
        glEnd();
    }

    // Linhas verticais
    for(int i = 0; i < AUTOMATON_HEIGHT; i++) {
        float cell_y = -1.0f + (i * CELL_SIZE);
        glBegin(GL_LINES);
        glVertex2f(1.0f, cell_y);
        glVertex2f(-1.0f, cell_y);
        glEnd();
    }
}

// Renderiza uma única célula na tela, de acordo com seu estado.
// Caso o estado não seja um dos estados do autômato de Greenber-Hastings,
// assumirá coloração de acordo com o estado de pausa da aplicação.
static void
render_grid_cell(int x_cell, int y_cell, int state)
{
    switch(state) {
    case 0:
        // Desnecessário renderizar uma célula em repouso
        return;
    case 1:
        // Células em recuperação são cinzas
        glColor3f(0.5f, 0.5f, 0.5f);
        break;
    case 2:
        // Células excitadas são brancas
        glColor3f(1.0f, 1.0f, 1.0f);
        break;
    default:
        // Estados diferentes destes indicam renderização
        // da posição do mouse
        if(input.paused) {
            // Cursor da aplicação pausada é vermelho
            glColor3f(0.6f, 0.0f, 0.0f);
        } else {
            // Na aplicação corrente, azul-claro
            glColor3f(0.0f, 0.4f, 0.6f);
        }
        break;
    }

    // Calcula a posição absoluta da célula, no plano cartesiano unitário
    double x = -1.0 + (x_cell * CELL_SIZE);
    double y =  1.0 - (y_cell * CELL_SIZE);

    // Renderiza a célula.
    // Veja que as células são renderizadas a partir do topo superior esquerdo.
    // X cresce para a direita; Y cresce para cima.
    glBegin(GL_TRIANGLES);
    
    // Triângulo 1
    glVertex2d(x, y);
    glVertex2d(x + CELL_SIZE, y);
    glVertex2d(x + CELL_SIZE, y - CELL_SIZE);

    // Triângulo 2
    glVertex2d(x + CELL_SIZE, y - CELL_SIZE);
    glVertex2d(x, y - CELL_SIZE);
    glVertex2d(x, y);

    glEnd();
}

// Renderiza, efetivamente, todo o autômato, levando em consideração as linhas
// na pausa, cada uma das células, e o cursor do mouse.
static void
render_grid()
{
    /* Linhas */
    // Linhas são renderizadas apenas quando a aplicação está pausada.
    if(input.paused) {
        render_grid_lines();
    }

    /* Células */
    // As células são renderizadas uma a uma.
    for(int y = 0; y < AUTOMATON_HEIGHT; y++) {
        for(int x = 0; x < AUTOMATON_WIDTH; x++) {
            render_grid_cell(x, y, cur_grid[y][x]);
        }
    }

    /* Cursor */
    // O cursor é renderizado como uma célula.
    render_grid_cell(input.cursor_grid_x, input.cursor_grid_y, 3);
}
