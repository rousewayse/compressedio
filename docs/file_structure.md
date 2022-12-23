\page fileformat A compressed file format page

## General storage file structure ##


|Structure | Member | Size in bytes | Description|
|:---------|:-------|:--------------|:-----------|
|Storage header| files\_table\_fpos | 8 | Position of stored compressed [files table](\ref cFILES) |
|^ | fbtable\_fpos| 8 | Position of [free blocks table](\ref cFBTABLE) | 
|^ | last\_served\_fpos | 8 | Position of last byte of storage file. | 
|^ | cdata\_size | 8 | Size in bytes of compressed data stored in storage. | 
|^ | free\_space\_size | 8 | Size in bytes of all free blocks in storage file. |
|^ | full\_blocks\_count | 8 | Amount of fully filled blocks stored in file. | 
|^ | partial\_blocks\_count | 8 | Amount of partially filled blocks in storage file | 
|^ | base\_block\_size | 8 | Configured [base uncompressed data size](\ref cCONFIG::base_block_size) of block. It can vary for different blocks.  | 
|^ | codec\_info\_position | 8 | Position of codec structure in storage file. |  
| Codec info | codec\_name | \f$N_{codec\_name} + 1\f$  | Serialized C-style codec name string with lenght \f$N_{codec\_name}\f$. |
|^ | codec\_options | \f$N_{options}\f$ | A serialized key/value pairs of [codec options](\ref cCONFIG::squash_codec_options) with size \f$N_{options}\f$ | 
| Files table | size | 8 | Amount of files etries in table. |  
|^ | allocated\_size | 8 | Amount of allocated entries in files table. | 
|^ | files\_headers\_fposes | \f$ \text{size} \cdot 8 \f$ | Sequentially stored positions of [files header structures](\ref cFILE_HEADER). | 
| Compressed file header | file\_name | \f$N_{file\_name} + 1\f$ | Serialized name of the compressed file with \f$N_{file\_name}\f$ bytes lenght. | 
|^ | btable\_fpos | 8 | Position in storage of [compressed blocks table](\ref cBTABLE) of file. | 
| Compressed/Free blocks table | size | 8 | Amount of [block](\ref cBLOCK) or [free block](\ref cFBLOCK) entries in table. | 
|^ | allocated\_size | 8 | Amount of allocated entries in table |
|^ | fposes | \f$ \text{size} \cdot 8\f$ | Sequentially stored positions of blocks. |
| Compressed data block | size | 8 | Size in bytes  of [decompressed block's data](\ref cBLOCK::data). | 
|^ | csize | 8 | Size in bytes of compressed block's data, i.e. stored in storage actually. | 
|^ |  mapped\_fpos | 8 |   Uncompressed file position asociated with this block's uncompressed data. | 
|^ | cdata | csize | Compressed data bytes. |
| Free block | size | 8 | Size in bytes of [free block](\ref cFBLOCK). | 
|^ | fdata | size | Bytes of storage file declared unused. |  

