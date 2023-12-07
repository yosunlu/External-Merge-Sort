typedef unsigned long long u8;
typedef struct
{
    u8  hi8;
    u8  lo8;
} u16;

struct summary
{
    u16 rec_count; /* total number of records */
    u16 checksum;  /* checksum of all records */
};