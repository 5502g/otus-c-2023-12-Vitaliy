#ifndef ZIP_H
#define ZIP_H

typedef unsigned char bool_t;
#define FALSE   0
#define TRUE    1

/*Central directory file header*/
struct cdfh {
    uint16_t version_made_by;
    uint16_t version_to_extract;
    uint16_t general_purpose_bit_flag;
    uint16_t compression_method;
    uint16_t modification_time;
    uint16_t modification_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
    uint16_t file_comment_length;
    uint16_t disk_number;
    uint16_t internal_file_attributes;
    uint32_t external_file_attributes;
    uint32_t local_file_header_offset;
} __attribute__((packed));
typedef struct cdfh cdfh_t;

/*End of central directory record*/
struct eocd {
    uint16_t disk_number;
    uint16_t start_disk_number;
    uint16_t number_central_directory_record;
    uint16_t total_central_directory_record;
    uint32_t size_of_central_directory;
    uint32_t central_directory_offset;
    uint16_t comment_length;
} __attribute__((packed));
typedef struct eocd eocd_t;

#endif /*ZIP_H*/
