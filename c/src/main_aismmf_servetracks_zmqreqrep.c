// ZeroMQ req-rep based server for track data.
//
// Provide the port and the aismmf file you want to serve data from
// as command line arguments.
//
// Protocol (message tuples):
//   req: <mmsi:string>
//   rep: <mmsi:string, messages_ndjson:string>
//   
// Server silently discards badly formed messages

#include <czmq.h>

#include "AISMmFile.h"


//  ----------------------------------------------------------------------
//  Help text

const char
*s_usage =
    "USAGE:\n"
    "    aismmf_servetracks_zmqreqrep AISMMF_FILE ZMQ_PORT";


//  ----------------------------------------------------------------------
//  Making JSON from tracks

static char *
makeTrackJson (MmsiTrack track) {
    char *jsonStr = NULL;

    for (const AisMsg *msg = MmsiTrack_begin (track),
                      *msg_END = MmsiTrack_end (track);
         msg != msg_END;
         ++msg)
    {
        char *newStr = zsys_sprintf
            ("%s%s{\"lat\": %f, \"lon\": %f, \"mmsi\": %d, \"timestamp\": %d, "
             "\"course\": %f, \"speed\": %f}",
             jsonStr == NULL ? "" : jsonStr,
             jsonStr == NULL ? "" : "\n",
             msg->lat, msg->lon, msg->mmsi, msg->timestamp,
             msg->course, msg->speed);
        zstr_free (&jsonStr);
        jsonStr = newStr;
    }

    return jsonStr;
}


//  ----------------------------------------------------------------------
//  Utils

// Parses the base 10 int in 'str' and writes the result to 'output'.
// Returns 0 on success or -1 on failure.
// Assume 'output' is modified in either case.
// TODO make more robust to errors
static int
parseInt (const char *str, int *output) {
    assert (str);
    
    long l = strtol (str, NULL, 10);
    if (l == 0 && !streq (str, "0")) {
        return -1;
    } else {
        *output = (int) l;
        return 0;
    }
}


//  ----------------------------------------------------------------------
//  main()

int
main (int argc, char **argv) {
    char *error = NULL;
    int ret = -1;  // function return value

    
    //  --------------------------------------------------
    //  Parse arguments

    if (argc != 3) {
        error = "Wrong number of arguments";
        goto die;
    }

    int port = -1;
    ret = parseInt (argv [2], &port);
    if (ret) {
        error = "Unparseable port number";
        goto die;
    }
    fprintf (stderr, "-- port is: %d\n", port);
    
    char *filepath = argv [1];

    
    //  --------------------------------------------------
    //  Open aismmf

    AISMmFile_t *mmf = AISMmFile_new (filepath);
    if (!mmf) {
        error = "Failed to open AISMmFile";
        goto die;
    }


    //  --------------------------------------------------
    //  Set up zmq rep port

    zsock_t *repsock = zsock_new (ZMQ_REP);
    assert (repsock);
    
    ret = zsock_bind (repsock, "tcp://127.0.0.1:%d", port);
    if (ret == -1) {
        error = "Failed to bind zmq req socket";
        goto die;
    }


    //  --------------------------------------------------
    //  Serve data until ctrl-c

    zpoller_t *poller = zpoller_new (repsock, NULL);
    assert (poller);

    while (! zsys_interrupted) {
        fprintf (stderr, "-- Waiting for message...\n");
        char *mmsi_str = NULL;

        // Wait for a message to arrive
        void *sock = zpoller_wait (poller, 10*1000);
        if (!sock)   // timeout
            goto cont;
        assert (sock == repsock);

        // Parse the mmsi string inside it
        mmsi_str = zstr_recv (repsock);
        fprintf (stderr, "-- Got req: %s\n", mmsi_str);
            
        int mmsi = -1;
        int ret = parseInt (mmsi_str, &mmsi);
        if (ret) {  // bad message; silently discard
            fprintf (stderr, "Bad request: %s\n", mmsi_str);
            goto cont;
        }

        // Send an empty response if we don't have data for that mmsi
        if (! AISMmFile_hasMmsi (mmf, mmsi)) {
            fprintf (stderr, "Req for non-present mmsi: %s\n", mmsi_str);
            zsock_send (repsock, "ss", mmsi_str, "");
            goto cont;
        }

        // All checks passed, so make the track and send it on
        
        MmsiTrack track = AISMmFile_mmsiTrack (mmf, mmsi);
        fprintf (stderr, "Found track length: %zu\n", MmsiTrack_size (track));
        //MmsiTrack_fprintln (track, stderr);
        
        char *trackJson = makeTrackJson (track);
        //fprintf (stderr, "Sending json reply: %s\n", trackJson);
        zsock_send (repsock, "ss", mmsi_str, trackJson);
        fprintf (stderr, "Sent reply\n");
        zstr_free (&trackJson);

    cont:
        zstr_free (&mmsi_str);
    }


    //  --------------------------------------------------
    //  All done, clean up and return

    zsock_destroy (&repsock);
    AISMmFile_destroy (&mmf);
    
    exit (0);
    

    //  -----------------------------------------------------------
    //  Error handler

 die:
    zsock_destroy (&repsock);
    AISMmFile_destroy (&mmf);
    
    fprintf (stderr, "=======================\n");
    fprintf (stderr, "Died on error: %s\n", error);
    fprintf (stderr, "%s\n", s_usage);
    fprintf (stderr, "=======================\n");
    
    exit (1);
}
