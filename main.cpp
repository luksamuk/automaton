/* ========================================================================== */
/*                                 main.cpp                                   */
/* ========================================================================== */
/*        Parte de uma implementação do autômato de Greenber-Hastings.
 *
 * Este projeto é software livre, distribuido sob a licença BSD 2-Clause.
 *
 * Copyright (C) 2018 Lucas Vieira
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <cstring>

/* Cabeçalho com definições gerais para o autômato, compartilhadas
 * entre demais partes do programa. */
#include "macros.hpp"

/* Cabeçalho com definições relacionadas à interface gráfica.
 * Estas definições foram separadas para garantir a legibilidade
 * deste arquivo. */
#include "window.hpp"

/* Grades do autômato, para um estado atual e um estado anterior */
int cur_grid[AUTOMATON_HEIGHT][AUTOMATON_WIDTH];
int old_grid[AUTOMATON_HEIGHT][AUTOMATON_WIDTH];


// Copia o estado atual do autômato para a matriz de backup.
void
copy_last_state()
{
    for(int i = 0; i < AUTOMATON_HEIGHT; i++) {
        memcpy(old_grid[i], cur_grid[i], AUTOMATON_WIDTH * sizeof(int));
    }
}

// Dada uma célula localizada em (x, y) no autômato, retorna o estado do
// vizinho desta célula na direção `direction`.
static int
get_neighbor_state(int x, int y, int direction)
{
    switch(direction) {
    case DIR_NORTH:
        y--;
        break;
    case DIR_SOUTH:
        y++;
        break;
    case DIR_WEST:
        x--;
        break;
    case DIR_EAST:
        x++;
        break;
    default: break;
    }

    if((x < 0) || (x >= AUTOMATON_WIDTH) || (y < 0) || (y >= AUTOMATON_HEIGHT)) {
        return CELL_RESTING;
    }

    return old_grid[y][x];
}

// Dada uma célula em (x, y), retorna a quantidade de vizinhos excitados que a
// mesma possui nas quatro direções da vizinhança de Von Neumann.
static int
get_excited_neighbors(int x, int y)
{
    int accumulator = 0;
    for(int i = DIR_NORTH; i <= DIR_EAST; i++) {
        if(get_neighbor_state(x, y, i) == CELL_EXCITED) {
            accumulator++;
        }
    }
    return accumulator;
}

// Aplica as regras do autômato de Greenber-Hastings no autômato.
// Esta função verifica o estado em `old_grid` e escreve diretamente
// em `cur_grid`.
void
apply_rules()
{
    for(int i = 0; i < AUTOMATON_HEIGHT; i++) {
        for(int j = 0; j < AUTOMATON_WIDTH; j++) {
            if(old_grid[i][j] == CELL_RESTING) {
                int n_neighbors = get_excited_neighbors(j, i);
                if(n_neighbors > 0) {
                    cur_grid[i][j] = CELL_EXCITED;
                }
            } else {
                cur_grid[i][j] = old_grid[i][j] - 1;
            }
        }
    }
}

// Inicializa o autômato, definindo todas as células em seu estado
// natural de descanso.
void
initialize_automata()
{
    for(int i = 0; i < AUTOMATON_HEIGHT; i++) {
        // memset(3) ocupa o destino com o byte informado.
        // Aqui, populamos cada sub-vetor com zeros, em uma quantidade
        // de bytes equivalente ao tamanho do vetor.
        memset(cur_grid[i], 0, AUTOMATON_WIDTH * sizeof(int));
    }
}

// Imprime a grade do estado atual do autômato no console.
// Esta função é utilizada na visualização não-gráfica.
static void
print_grid()
{
    for(int i = 0; i < AUTOMATON_HEIGHT; i++) {
        std::cout << '|';
        for(int j = 0; j < AUTOMATON_WIDTH; j++) {
            switch(cur_grid[i][j]) {
            case CELL_RESTING:
                std::cout << ' ';
                break;
            case CELL_RECOVER:
                std::cout << 'x';
                break;
            case CELL_EXCITED:
                std::cout << 'o';
            default: break;
            }
        }
        std::cout << '|' << std::endl;
    }
}

// Realiza os ciclos das gerações do autômato, mostrando-o no console.
// Esta função é utilizada na visualização não-gráfica.
void
automata_console_loop()
{
    // Debug: coloca uma célula com estado excitado bem no centro.
    cur_grid[AUTOMATON_HEIGHT / 2][AUTOMATON_WIDTH / 2] = CELL_EXCITED;
    
    while(true) {
        print_grid();
        copy_last_state();
        apply_rules();

        // Interrupção: basta que o usuário digite 'q'.
        if(getchar() == 'q') {
            break;
        }
    }
}

int
handle_args(int argc, char** argv)
{
    // Valores de retorno:
    // 0: A aplicação corre normalmente.
    // 1: A aplicação corre em console.
    // 2: A aplicação sai imediatamente.

    bool nogui = false;

    /*
     * Este loop tem duas funções:
     * - Verificar se o argumento -nogui foi passado
     *   pelo console. Se sim, apenas mostrará saída
     *   textual;
     * - Verificar se o argumento -help foi passado
     *   pelo console. Se sim, mostrará texto de ajuda
     *   e encerrará o programa.
    */
    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "--nogui")) {
            nogui = true;
        } else if(!strcmp(argv[i], "--help")) {
            std::cout << "Greenber-Hastings Automaton"      << std::endl
                      << "Copyright (C) 2018 Lucas Vieira"  << std::endl
                      << "This program is distributed under the BSD-2 License. "
                      << "See source code for details."
                      << std::endl << std::endl
                
                      << "Command line args:"               << std::endl
                      << "\t--help           \tShow this help prompt."
                      << std::endl
                      << "\t--nogui          \tForce execution of automata on "
                      << "console."
                      << std::endl << std::endl
                
                      << "Runtime GUI commands:" << std::endl
                      << "\tc                \tClear the grid" << std::endl
                      << "\tLeft mouse button\t"
                      << "Excite highlighted cell" << std::endl
                      << "\tRight mouse button\t"
                      << "Pause/unpause application"
                      << std::endl << std::endl

                      << "Runtime CLI commands:" << std::endl
                      << "\tEnter            \tIterate or input command"
                      << std::endl
                      << "\tq                \tFinish simulation on input"
                      << std::endl << std::endl;
            return 2;
        }
    }

    if(nogui) {
        return 1;
    }

    return 0;
}

// Ponto de entrada da aplicação.
int
main(int argc, char** argv)
{
    // Inicializa a aplicação, iterando sobre os argumentos de console
    // fornecidos.
    int arg_handler = handle_args(argc, argv);

    if(arg_handler == 2) {
        // A opção '2' indica que o texto de ajuda foi impresso e,
        // portanto, o programa se encerra em seguida.
        return 0;
    }

    // Inicializa o autômato
    initialize_automata();

    if((arg_handler == 1) || !create_window()) {
        // Em caso de indicador de modo console ou falha ao criar a janela,
        // dê fallback para o modo texto
        automata_console_loop();
    } else {
        // Caso contrário, execute a aplicação normalmente
        automata_gui_loop();
    }
    
    return 0;
}
