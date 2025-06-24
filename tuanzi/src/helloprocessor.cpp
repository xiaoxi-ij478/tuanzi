#include "all.h"
#include "helloprocessor.h"

CHelloProcessor::CHelloProcessor() : hello_thread()
{}

CHelloProcessor::~CHelloProcessor()
{}


void CHelloProcessor::Exit() const
{
    if (!hello_thread)
        return;

    hello_thread->PostThreadMessage(STOP_THREAD_MTYPE, 0, nullptr);
    hello_thread = nullptr;
}

void CHelloProcessor::ProcessorRun(
    int msgid,
    unsigned hello_timer_interval,
    unsigned hello_para
) const
{
    if (hello_thread) {
        g_log_Wireless.AppendText("Post Hello Thread  WM_ChangeHelloPara");
        hello_thread->PostThreadMessage(
            CHANGE_HELLOPARA_MTYPE,
            hello_timer_interval,
            reinterpret_cast<void *>(hello_para)
        );

    } else {
        hello_thread = new CHelloThread;
        hello_thread->msgid = msgid;
        hello_thread->hello_timer_interval = hello_timer_interval;
        hello_thread->hello_para = hello_para;
        hello_thread->CreateThread(nullptr, false);
        hello_thread->StartThread();

        if (
            !hello_thread->PostThreadMessage(
                SET_HELLOTIMER_PLEASE_MTYPE,
                0,
                nullptr
            )
        )
            g_log_Wireless.AppendText(
                "CHelloThread PostThreadMessage WM_SetHelloTimerPlease failed"
            );
    }
}

void CHelloProcessor::ProcessorStop() const
{
    if (!hello_thread)
        return;

    hello_thread->PostThreadMessage(STOP_THREAD_MTYPE, 0, nullptr);
    hello_thread = nullptr;
}
