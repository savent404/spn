#include <lwip/timeouts.h>
#include <spn/led.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

struct led_ctx {
  int cnt;
  char val;
  uint32_t dur;
};

static void led_blink_handler(void* arg) {
  struct led_ctx* ctx = (struct led_ctx*)arg;
  int fd;

  fd = open("/sys/class/leds/ACT/brightness", O_WRONLY);
  if (fd != -1) {
      if (write(fd, &ctx->val, sizeof(ctx->val)) == -1) {
          perror("write");
      }
      close(fd);
  } else {
      perror("open");
  }

  if (ctx->val == '0') {
    ctx->val = '1';
  } else {
    ctx->val = '0';
  }

  if (--ctx->cnt) {
    sys_timeout(ctx->dur, led_blink_handler, ctx);
  }
}

void spn_port_led_flash(void) {
  static struct led_ctx ctx = {.dur = 500};

  ctx.cnt = 4;
  ctx.val = '1';
  sys_untimeout(led_blink_handler, &ctx);
  sys_timeout(0, led_blink_handler, &ctx);
}
