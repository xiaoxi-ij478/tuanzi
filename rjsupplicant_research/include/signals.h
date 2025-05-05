#ifndef SIGNALS_H_INCLUDED
#define SIGNALS_H_INCLUDED

void set_signals();
void release_signals();
void hold_signals();
void sig_quit(int);
void sig_tstp(int);

#endif // SIGNALS_H_INCLUDED
