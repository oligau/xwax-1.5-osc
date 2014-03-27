#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "lo/lo.h"

#include "osc.h"
#include "debug.h"

int done = 0;
lo_server_thread st;

void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

int status_handler(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data, void *user_data)
{
    printf("deck: %i\n"
            "path: %s\n"
            "artist: %s\n"
            "title: %s\n"
            "length: %f\n"
            "position: %f\n"
            "pitch: %f\n"
            "timecode_control: %i\n",
            argv[0]->i,
            &argv[1]->s,
            &argv[2]->s,
            &argv[3]->s,
            argv[4]->f,
            argv[5]->f,
            argv[6]->f,
            argv[7]->i);
    
    done = 1;
    
    return 0;
}

void osc_start_server()
{
    /* start a new server on port 7770 */
    st = lo_server_thread_new("7771", error);
    
    lo_server_thread_add_method(st, "/xwax/status", "isssfffi", status_handler, NULL);
    
    lo_server_thread_start(st);
}

int osc_send_get_status(char *server, int d)
{
    done = 0;
    osc_start_server();
    
    lo_address t = lo_address_new(server, "7770");
    
    if (lo_send(t, "/xwax/get_status", "i", d) == -1) {
        fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
            lo_address_errstr(t));
        return 1;
    }

    while (!done) {
        usleep(1000);
    }

    lo_server_thread_free(st);
    
    return 0;
}

int osc_send_load_track(char *server, int d, char *path, char *artist, char *title)
{
    lo_address t = lo_address_new(server, "7770");
    
    if (lo_send(t, "/xwax/load_track", "isss", d, path, artist, title) == -1) {
        fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
            lo_address_errstr(t));
        return 1;
    }
    
    return 0;
}

int osc_send_set_cue(char *server, int d, int q, double pos)
{
    lo_address t = lo_address_new(server, "7770");
    
    if (lo_send(t, "/xwax/set_cue", "iif", d, q, pos) == -1) {
        fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
            lo_address_errstr(t));
        return 1;
    }
    
    return 0;
}

int osc_send_punch_cue(char *server, int d, int q)
{
    lo_address t = lo_address_new(server, "7770");
    
    if (lo_send(t, "/xwax/punch_cue", "ii", d, q) == -1) {
        fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
            lo_address_errstr(t));
        return 1;
    }
    
    return 0;
}

static void usage(FILE *fd)
{
    fprintf(fd, "Usage: xwax-client <server> load_track <deck-number> "
            "<pathname> <artist> <title>\n"
            "xwax-client <server> set_cue <deck-number> "
            "<cue-number> <position>\n"
            "xwax-client <server> punch_cue <deck-number> "
            "<cue-number>\n"
            "xwax-client <server> get_status <deck-number>\n");     
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage(stderr);
        return 0;
    }
    
    if(strcmp(argv[2], "load_track") == 0) {
        if (argc < 7) {
            usage(stderr);
            return 0;
        }        
        
        int d;
        char *server;
        char *path;
        char *artist;
        char *title;
        
        server = argv[1];
        d = atoi(argv[3]);
        path = argv[4];
        artist = argv[5];
        title = argv[6];
        
        return osc_send_load_track(server, d, path, artist, title);
        
    }else if(strcmp(argv[2], "set_cue") == 0) {
        if (argc < 6) {
            usage(stderr);
            return 0;
        }        
        
        int d, q;
        char *server;
        double pos;
        
        server = argv[1];
        d = atoi(argv[3]);
        q = atoi(argv[4]);
        pos = atof(argv[5]);
        
        return osc_send_set_cue(server, d, q, pos);        
        
    }else if(strcmp(argv[2], "punch_cue") == 0) {
        if (argc < 5) {
            usage(stderr);
            return 0;
        }        
        
        int d, q;
        char *server;
                
        server = argv[1];
        d = atoi(argv[3]);
        q = atoi(argv[4]);
        
        return osc_send_punch_cue(server, d, q);        
        
    }else if(strcmp(argv[2], "get_status") == 0) {
        if (argc < 4) {
            usage(stderr);
            return 0;
        }        
        
        int d;
        char *server;
                
        server = argv[1];
        d = atoi(argv[3]);
        
        return osc_send_get_status(server, d)
;        
        
    }
        
    return 0;

}
