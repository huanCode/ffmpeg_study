typedef struct MOVTrack {
    AVStream        *st;
    AVCodecParameters *par;
    MOVIentry   *cluster;
    AVFormatContext *rtp_ctx;
    HintSampleQueue sample_queue;
    AVIOContext *mdat_buf;
    MOVFragmentInfo *frag_info;
    MOVMuxCencContext cenc;
} MOVTrack;


typedef struct AVStream {
    int index;
    int id;
    AVCodecContext *codec;
    AVDictionary *metadata;
    AVPacketSideData *side_data;
    enum AVStreamParseType need_parsing;
    struct AVCodecParserContext *parser;
    struct AVPacketList *last_in_packet_buffer;
    AVProbeData probe_data;
    AVIndexEntry *index_entries;
    AVStreamInternal *internal;
    AVCodecParameters *codecpar;
} AVStream;


typedef struct MOVIentry {
    uint64_t     pos;
    int64_t      dts;
    unsigned int size;
    unsigned int samples_in_chunk;
    unsigned int chunkNum;              ///< Chunk number if the current entry is a chunk start otherwise 0
    unsigned int entries;
    int          cts;
#define MOV_SYNC_SAMPLE         0x0001
#define MOV_PARTIAL_SYNC_SAMPLE 0x0002
    uint32_t     flags;
} MOVIentry;

typedef struct AVIOContext {
    /**
     * A class for private options.
     *
     * If this AVIOContext is created by avio_open2(), av_class is set and
     * passes the options down to protocols.
     *
     * If this AVIOContext is manually allocated, then av_class may be set by
     * the caller.
     *
     * warning -- this field can be NULL, be sure to not pass this AVIOContext
     * to any av_opt_* functions in that case.
     */
    const AVClass *av_class;

    /*
     * The following shows the relationship between buffer, buf_ptr, buf_end, buf_size,
     * and pos, when reading and when writing (since AVIOContext is used for both):
     *
     **********************************************************************************
     *                                   READING
     **********************************************************************************
     *
     *                            |              buffer_size              |
     *                            |---------------------------------------|
     *                            |                                       |
     *
     *                         buffer          buf_ptr       buf_end
     *                            +---------------+-----------------------+
     *                            |/ / / / / / / /|/ / / / / / /|         |
     *  read buffer:              |/ / consumed / | to be read /|         |
     *                            |/ / / / / / / /|/ / / / / / /|         |
     *                            +---------------+-----------------------+
     *
     *                                                         pos
     *              +-------------------------------------------+-----------------+
     *  input file: |                                           |                 |
     *              +-------------------------------------------+-----------------+
     *
     *
     **********************************************************************************
     *                                   WRITING
     **********************************************************************************
     *
     *                                          |          buffer_size          |
     *                                          |-------------------------------|
     *                                          |                               |
     *
     *                                       buffer              buf_ptr     buf_end
     *                                          +-------------------+-----------+
     *                                          |/ / / / / / / / / /|           |
     *  write buffer:                           | / to be flushed / |           |
     *                                          |/ / / / / / / / / /|           |
     *                                          +-------------------+-----------+
     *
     *                                         pos
     *               +--------------------------+-----------------------------------+
     *  output file: |                          |                                   |
     *               +--------------------------+-----------------------------------+
     *
     */
    unsigned char *buffer;  /**< Start of the buffer. */
    int buffer_size;        /**< Maximum buffer size */
    unsigned char *buf_ptr; /**< Current position in the buffer */
    unsigned char *buf_end; /**< End of the data, may be less than
                                 buffer+buffer_size if the read function returned
                                 less data than requested, e.g. for streams where
                                 no more data has been received yet. */
    void *opaque;           /**< A private pointer, passed to the read/write/seek/...
                                 functions. */
    int (*read_packet)(void *opaque, uint8_t *buf, int buf_size);   //
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);
    int64_t (*seek)(void *opaque, int64_t offset, int whence);
    int64_t pos;            /**< position in the file of the current buffer */
    int must_flush;         /**< true if the next seek should flush */
    int eof_reached;        /**< true if eof reached */
    int write_flag;         /**< true if open for writing */
    int max_packet_size;
    unsigned long checksum;
    unsigned char *checksum_ptr;
    unsigned long (*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
    int error;              /**< contains the error code or 0 if no error happened */
    /**
     * Pause or resume playback for network streaming protocols - e.g. MMS.
     */
    int (*read_pause)(void *opaque, int pause);
    /**
     * Seek to a given timestamp in stream with the specified stream_index.
     * Needed for some network streaming protocols which don't support seeking
     * to byte position.
     */
    int64_t (*read_seek)(void *opaque, int stream_index,
                         int64_t timestamp, int flags);
    /**
     * A combination of AVIO_SEEKABLE_ flags or 0 when the stream is not seekable.
     */
    int seekable;

    /**
     * max filesize, used to limit allocations
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int64_t maxsize;

    /**
     * avio_read and avio_write should if possible be satisfied directly
     * instead of going through a buffer, and avio_seek will always
     * call the underlying seek function directly.
     */
    int direct;

    /**
     * Bytes read statistic
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int64_t bytes_read;

    /**
     * seek statistic
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int seek_count;

    /**
     * writeout statistic
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int writeout_count;

    /**
     * Original buffer size
     * used internally after probing and ensure seekback to reset the buffer size
     * This field is internal to libavformat and access from outside is not allowed.
     */
    int orig_buffer_size;

    /**
     * Threshold to favor readahead over seek.
     * This is current internal only, do not use from outside.
     */
    int short_seek_threshold;

    /**
     * ',' separated list of allowed protocols.
     */
    const char *protocol_whitelist;

    /**
     * ',' separated list of disallowed protocols.
     */
    const char *protocol_blacklist;

    /**
     * A callback that is used instead of write_packet.
     */
    int (*write_data_type)(void *opaque, uint8_t *buf, int buf_size,
                           enum AVIODataMarkerType type, int64_t time);
    /**
     * If set, don't call write_data_type separately for AVIO_DATA_MARKER_BOUNDARY_POINT,
     * but ignore them and treat them as AVIO_DATA_MARKER_UNKNOWN (to avoid needlessly
     * small chunks of data returned from the callback).
     */
    int ignore_boundary_point;

    /**
     * Internal, not meant to be used from outside of AVIOContext.
     */
    enum AVIODataMarkerType current_type;
    int64_t last_time;

    /**
     * A callback that is used instead of short_seek_threshold.
     * This is current internal only, do not use from outside.
     */
    int (*short_seek_get)(void *opaque);
} AVIOContext;