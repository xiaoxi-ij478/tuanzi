#ifndef SIGNALS_H_INCLUDED
#define SIGNALS_H_INCLUDED

extern void set_signals();
extern void release_signals();
extern void hold_signals();
extern void sig_quit(int);
extern void sig_tstp(int);

#endif // SIGNALS_H_INCLUDED
