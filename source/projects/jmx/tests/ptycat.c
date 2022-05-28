/* ptycat (ptypipe? ptypair?)
 *
 * create a pair of pseudo-terminal slaves connected to each other
 *
 * Link with -lutil
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <util.h>

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

/*
  (void)ioctl(STDIN_FILENO, TIOCGWINSZ, &win);

*/

/* TODO: make symlinks, unlink on atexit */

static uint8_t buf[BUFSIZ]; /* BUFSIZ from stdio.h, at least 256 */
static char *log_dir = NULL;

void logdata (char *dir, uint8_t *data, int n) {
  if (dir != log_dir) fprintf (stdout, "\n%s", dir);
  log_dir = dir;
  for (; n > 0; n--, data++) fprintf (stdout, " %02x", *data);
  fflush (stdout);
}

int main (int argc, char* argv[])
{
  char name[256]; /* max namelen = 255 for most fs. */
  fd_set rfd;

  struct termios tt;
  struct winsize ws;
  int master[2], slave[2];
  int n, nfds, cc;

  if (tcgetattr (STDIN_FILENO, &tt) < 0)
  {
    perror("Cannot get terminal attributes of stdin");
    exit(1);
  }
  cfmakeraw (&tt);
  for (int i = 0; i < 2; i++)
  {
    if (openpty (&master[i], &slave[i], name, &tt, NULL /*ws*/) < 0)
    {
      perror("Cannot open pty");
      exit(1);
    }
    puts(name);
  }

  for (;;) {
    FD_ZERO(&rfd);  
    FD_SET(master[0], &rfd);
    FD_SET(master[1], &rfd);
    nfds = max(master[0], master[1]) + 1;
    n = select(nfds, &rfd, 0, 0, NULL);
    if (n > 0 || errno == EINTR)
    {
      if (FD_ISSET(master[0], &rfd))
      {
    if ((cc = read(master[0], buf, sizeof(buf))) > 0)
    {
      (void) write(master[1], buf, cc);
      logdata (">>>", buf, cc);
    }
      }

      if (FD_ISSET(master[1], &rfd))
      {
    if ((cc = read(master[1], buf, sizeof(buf))) > 0)
    {
      (void) write(master[0], buf, cc);
      logdata ("<<<", buf, cc);
    }
      }
    }
  }
  /*    This never reached */
  return 0; 
}

