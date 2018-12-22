#ifndef AUTOMATON_WINDOW_HPP
#define AUTOMATON_WINDOW_HPP

/* Este cabeçalho exporta apenas funções a serem utilizadas no arquivo
 * `main.cpp` e, portanto, foi escrito minimamente. Para implementações e
 * detalhes, veja `window.cpp`. */

#define WINDOW_REFRESH_INTERVAL 0.25

bool create_window();
void automata_gui_loop();

#endif
