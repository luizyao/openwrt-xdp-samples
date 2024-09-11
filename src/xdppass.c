#include "xdppass.skel.h"
#include <bpf/libbpf.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

static int libbpf_print_fn(enum libbpf_print_level level, const char *format,
                           va_list args) {
  return vfprintf(stderr, format, args);
}

int main(int argc, char **argv) {
  unsigned int ifindex;

  if (argc != 2) {
    fprintf(stderr, "Interface name is required!\n");
    return 1;
  }

  ifindex = if_nametoindex(argv[1]);

  struct xdppass_bpf *skel;
  int err;

  /* Set up libbpf errors and debug info callback */
  libbpf_set_print(libbpf_print_fn);

  /* Open BPF application */
  skel = xdppass_bpf__open();
  if (!skel) {
    fprintf(stderr, "Failed to open BPF skeleton\n");
    return 1;
  }

  /* Load & verify BPF programs */
  err = xdppass_bpf__load(skel);
  if (err) {
    fprintf(stderr, "Failed to load and verify BPF skeleton\n");
    goto cleanup;
  }

  // /* Attach tracepoint handler */
  // err = xdppass_bpf__attach(skel);
  // if (err) {
  // 	fprintf(stderr, "Failed to attach BPF skeleton\n");
  // 	goto cleanup;
  // }

  /* Attach xdp program to interface */
  struct bpf_link *link =
      bpf_program__attach_xdp(skel->progs.xdp_pass, ifindex);
  if (!link) {
    fprintf(stderr, "Failed to attach xdp program to interface index %d\n",
            ifindex);
    goto cleanup;
  }

  printf("Successfully started! Please run `sudo cat "
         "/sys/kernel/debug/tracing/trace_pipe` "
         "to see output of the BPF programs.\n");

  for (;;) {
    /* trigger our BPF program */
    fprintf(stderr, ".");
    sleep(1);
  }

cleanup:
  xdppass_bpf__destroy(skel);
  return -err;
}
