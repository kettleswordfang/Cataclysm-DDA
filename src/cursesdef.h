#pragma once
#ifndef CURSESDEF_H
#define CURSESDEF_H

#include "string_formatter.h"

#include <memory>
#include <cstdint>
#include <string>

class nc_color;

/**
 * Contains the curses interface used by the whole game.
 *
 * All code (except platform/OS/build type specific code) should use functions/
 * types from this namespace only. The @ref input_manager and @ref input_context
 * should be used for user input.
 *
 * There are currently (Nov 2017) two implementations for most of this interface:
 * - ncurses (mostly in ncurses_def.cpp). The interface originates from there,
 *   so it's mostly just forwarding to ncurses functions of the same name.
 * - our own curses library @ref cata_cursesport (mostly in cursesport.cpp),
 *   cursesport.h contains the structures and some functions specific to that.
 *
 * A few (system specific) functions are implemented in three versions:
 * - ncurses (ncurses_def.cpp),
 * - Windows curses (no SDL, using @ref cata_cursesport, see wincurses.cpp), and
 * - SDL curses (using @ref cata_cursesport, see sdltiles.cpp).
 *
 * As this interface is derived from ncurses, refer to documentation of that.
 *
 * The interface is in a separate namespace we can link with the ncurses library,
 * which exports its functions globally.
 */
//Currently this namespace is automatically exported into the global namespace to
//allow existing code (that called global ncurses functions) to remain unchanged.
//The compiler will translate `WINDOW *win = newwin(...)` into
//`catacurses::WINDWO *win = catacurses::newwin(...)`
namespace catacurses
{

/// @throws std::exception upon any errors. The caller should display / log it
/// and abort the program. Only continue the program when this returned normally.
void init_interface();

// it's void because than it's compatible with ncurses and with our own curses
// library becaue void* can be converted to either
//@todo phase this out. Store window objects everywhere instead of WINDOW pointers
using WINDOW = void;

/**
 * A simple wrapper over `WINDOW*`.
 * Currently it does not do anything at all. It is implicitly constructed
 * from a pointer and implicitly converted to it.
 * Because all curses function here receive/return a `window` (and not a
 * pointer), it allows callers to store the `window` as pointer (like
 * it's done all over the place), and it allows to forward a pointer to
 * the functions.
 * The implementation of the curses interface can cast the pointer as they need.
 */
class window
{
    private:
        WINDOW *native_window;

    public:
        window() : native_window( nullptr ) { }
        template<typename T>
        window( T *const ptr ) : native_window( static_cast<WINDOW *>( ptr ) ) {
        }
        ~window() {
        }
        template<typename T = WINDOW>
        T * get() const {
            return static_cast<T *>( native_window );
        }
        operator WINDOW *() const {
            return get();
        }
};

struct delwin_functor {
    void operator()( WINDOW *w ) const;
};
/**
 * A Wrapper around the WINDOW pointer, it automatically deletes the
 * window (see delwin_functor) when the variable gets out of scope.
 * This includes calling werase, wrefresh and delwin.
 * Usage:
 * 1. Acquire a WINDOW pointer via @ref newwin like normal, store it in a pointer variable.
 * 2. Create a variable of type WINDOW_PTR *on the stack*, initialize it with the pointer from 1.
 * 3. Do the usual stuff with window, print, update, etc. but do *not* call delwin on it.
 * 4. When the function is left, the WINDOW_PTR variable is destroyed, and its destructor is called,
 *    it calls werase, wrefresh and most importantly delwin to free the memory associated wit the pointer.
 * To trigger the delwin call earlier call some_window_ptr.reset().
 * To prevent the delwin call when the function is left (because the window is already deleted or, it should
 * not be deleted), call some_window_ptr.release().
 */
using WINDOW_PTR = std::unique_ptr<WINDOW, delwin_functor>;

enum base_color : short {
    black = 0x00,    // RGB{0, 0, 0}
    red = 0x01,      // RGB{196, 0, 0}
    green = 0x02,    // RGB{0, 196, 0}
    yellow = 0x03,   // RGB{196, 180, 30}
    blue = 0x04,     // RGB{0, 0, 196}
    magenta = 0x05,  // RGB{196, 0, 180}
    cyan = 0x06,     // RGB{0, 170, 200}
    white = 0x07,    // RGB{196, 196, 196}
};

using chtype = int;
using attr_t = unsigned short;

extern window stdscr;

window newwin( int nlines, int ncols, int begin_y, int begin_x );
void delwin( const window &win );
void wborder( const window &win, chtype ls, chtype rs, chtype ts, chtype bs, chtype tl, chtype tr,
              chtype bl, chtype br );
void mvwhline( const window &win, int y, int x, chtype ch, int n );
void mvwvline( const window &win, int y, int x, chtype ch, int n );
void wrefresh( const window &win );
void refresh();
void wredrawln( const window &win, int beg_line, int num_lines );

void mvwprintw( const window &win, int y, int x, const std::string &text );
template<typename ...Args>
inline void mvwprintw( const window &win, const int y, const int x, const char *const fmt,
                       Args &&... args )
{
    return mvwprintw( win, y, x, string_format( fmt, std::forward<Args>( args )... ) );
}

void wprintw( const window &win, const std::string &text );
template<typename ...Args>
inline void wprintw( const window &win, const char *const fmt, Args &&... args )
{
    return wprintw( win, string_format( fmt, std::forward<Args>( args )... ) );
}

void werase( const window &win );
void init_pair( short pair, base_color f, base_color b );
void wmove( const window &win, int y, int x );
void clear();
void erase();
void endwin();
void mvwaddch( const window &win, int y, int x, const chtype ch );
void wclear( const window &win );
void curs_set( int visibility );
void wattron( const window &win, const nc_color &attrs );
void wattroff( const window &win, int attrs );
void waddch( const window &win, const chtype ch );
int getmaxy( const window &win );
int getmaxx( const window &win );
int getbegx( const window &win );
int getbegy( const window &win );
int getcurx( const window &win );
int getcury( const window &win );
} // namespace catacurses

//@todo move "using namespace" into the cpp/header files that include this file
//see note at start of namepace catacurses
#ifndef CATACURSES_DONT_USE_NAMESPACE_CATACURSES
using namespace catacurses;
#endif

#endif
