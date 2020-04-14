#include <kvzrtp/lib.hh>
#include <kvzrtp/clock.hh>
#include <cstring>
#include<algorithm> 
#include <easy/profiler.h>

struct thread_info {
    size_t pkts;
    size_t bytes;
    std::chrono::high_resolution_clock::time_point start;
} *thread_info;

std::atomic<int> nready(0);

void hook(void *arg, kvz_rtp::frame::rtp_frame *frame)
{
    int tid = *(int *)arg;

    if (thread_info[tid].pkts == 0)
        thread_info[tid].start = std::chrono::high_resolution_clock::now();

    /* receiver returns NULL to indicate that it has not received a frame in 10s 
     * and the sender has likely stopped sending frames long time ago so the benchmark 
     * can proceed to next run and ma*/
    if (!frame) {
        fprintf(stderr, "discard %zu %lu\n", thread_info[tid].bytes, thread_info[tid].pkts);
        nready++;
        /* return; */
        while (1)
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        /* exit(EXIT_FAILURE); */
    }

    thread_info[tid].bytes += frame->payload_len;

    (void)kvz_rtp::frame::dealloc_frame(frame);

    if (++thread_info[tid].pkts == 602) {
        fprintf(stderr, "%zu %lu\n", thread_info[tid].bytes,
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - thread_info[tid].start
            ).count()
        );
        nready++;
        /* exit(EXIT_SUCCESS); */
    }
}

void thread_func(int thread_num)
{
    kvz_rtp::context rtp_ctx;

    auto sess = rtp_ctx.create_session("10.21.25.200");
    auto hevc = sess->create_stream(
        8888 + thread_num,
        8889 + thread_num,
        RTP_FORMAT_HEVC,
        0
    );

    int tid = thread_num / 2;

    hevc->install_receive_hook(&tid, hook);

    for (;;)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

    rtp_ctx.destroy_session(sess);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: ./%s <number of threads>\n", __FILE__);
        return -1;
    }

    int nthreads = atoi(argv[1]);
    thread_info  = (struct thread_info *)calloc(nthreads, sizeof(*thread_info));

    for (int i = 0; i < nthreads; ++i)
        new std::thread(thread_func, i * 2);

    while (nready.load() != nthreads)
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
}
