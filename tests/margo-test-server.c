/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <mercury.h>
#include <abt.h>
#include <margo.h>

#include "my-rpc.h"

/* example server program.  Starts margo, registers the example RPC type,
 * and then executes indefinitely.
 */

struct options
{
    int single_pool_mode;
    char *hostfile;
    char *listen_addr;
};

static void parse_args(int argc, char **argv, struct options *opts);

int main(int argc, char **argv) 
{
    hg_return_t hret;
    margo_instance_id mid;
    struct options opts;

    parse_args(argc, argv, &opts);

    /* actually start margo -- this step encapsulates the Mercury and
     * Argobots initialization and must precede their use */
    /* If single pool mode, use the calling xstream to drive progress and
     * execute handlers. If not, use a dedicated progress xstream and
     * run handlers directly on the calling xstream
     */
    /***************************************/
    if(opts.single_pool_mode)
        mid = margo_init(opts.listen_addr, MARGO_SERVER_MODE, 0, -1);
    else
        mid = margo_init(opts.listen_addr, MARGO_SERVER_MODE, 1, 0);
    if(mid == MARGO_INSTANCE_NULL)
    {
        fprintf(stderr, "Error: margo_init()\n");
        return(-1);
    }

    if(opts.hostfile)
    {
        FILE *fp;
        hg_addr_t addr_self;
        char addr_self_string[128];
        hg_size_t addr_self_string_sz = 128;

        /* figure out what address this server is listening on */
        hret = margo_addr_self(mid, &addr_self);
        if(hret != HG_SUCCESS)
        {
            fprintf(stderr, "Error: margo_addr_self()\n");
            margo_finalize(mid);
            return(-1);
        }
        hret = margo_addr_to_string(mid, addr_self_string, &addr_self_string_sz, addr_self);
        if(hret != HG_SUCCESS)
        {
            fprintf(stderr, "Error: margo_addr_to_string()\n");
            margo_addr_free(mid, addr_self);
            margo_finalize(mid);
            return(-1);
        }
        margo_addr_free(mid, addr_self);

        fp = fopen(opts.hostfile, "w");
        if(!fp)
        {
            perror("fopen");
            margo_finalize(mid);
            return(-1);
        }

        fprintf(fp, "%s", addr_self_string);
        fclose(fp);
    }

    /* register RPC */
    MARGO_REGISTER(mid, "my_rpc", my_rpc_in_t, my_rpc_out_t, 
        my_rpc_ult);
    MARGO_REGISTER(mid, "my_rpc_hang", my_rpc_hang_in_t, my_rpc_hang_out_t, 
        my_rpc_hang_ult);
    MARGO_REGISTER(mid, "my_shutdown_rpc", void, void, 
        my_rpc_shutdown_ult);

    /* NOTE: at this point this server ULT has two options.  It can wait on
     * whatever mechanism it wants to (however long the daemon should run and
     * then call margo_finalize().  Otherwise, it can call
     * margo_wait_for_finalize() on the assumption that it should block until
     * some other entity calls margo_finalize().
     *
     * This example does the latter.  Margo will be finalized by a special
     * RPC from the client.
     *
     * This approach will allow the server to idle gracefully even when
     * executed in "single" mode, in which the main thread of the server
     * daemon and the progress thread for Mercury are executing in the same
     * ABT pool.
     */
    margo_wait_for_finalize(mid);

    return(0);
}

static void usage(int argc, char **argv)
{
    fprintf(stderr, "Usage: %s listen_address [-s] [-f filename]\n",
        argv[0]);
    fprintf(stderr, "   listen_address is the address or protocol for the server to use\n");
    fprintf(stderr, "   [-s] for single pool mode\n");
    fprintf(stderr, "   [-f filename] to write the server address to a file\n");
    return;
}


static void parse_args(int argc, char **argv, struct options *opts)
{
    int opt;
    
    memset(opts, 0, sizeof(*opts));

    while((opt = getopt(argc, argv, "f:s")) != -1)
    {
        switch(opt)
        {
            case 's':
                opts->single_pool_mode = 1;
                break;
            case 'f':
                opts->hostfile = strdup(optarg);
                break;
            default: 
                usage(argc, argv);
                exit(EXIT_FAILURE);
        }
    }

    if(optind >= argc)
    {
        usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    opts->listen_addr = strdup(argv[optind]);
    
    return;
}
