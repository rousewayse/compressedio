/*! \file compressedio.h
 *  \brief A user header file.
 */
#ifndef _COMPRESSEDIO_H
#define _COMPRESSEDIO_H

#include <stdio.h>
#include <squash/squash.h>
#ifdef __cplusplus
extern "C"{
#endif
	/*! \defgroup cstorage cSTORAGE 
	 *	\brief Functions and data structures asociated with storage file.
	 */
	
	/*! \defgroup cconfig cCONFIG 
	 * \brief Library behaviour configuration.
	 */
	/*! \defgroup cfile cFILE
	 * \brief Functions and data structures asociated with compressed file in a storage.
	 */
	/*! \defgroup cfile_io IO Functions 
	 * \ingroup cfile */
	/*! \defgroup cfile_fpos Positioning functions
	 * \ingroup cfile */
	
	/*cSTORAGE structure represents an archive stored in filesysem as a file.*/
	struct cSTORAGE;
   	/*!\ingroup cstorage 
	 * \brief typedef of struct cSTORAGE. */
	typedef struct cSTORAGE cSTORAGE;

	/*cFILE structure represents a data stream, i.e file, stored in multistream archive cSTORAGE*/
	struct cFILE;
	/*!\ingroup cfile 
	 * \brief typedef of struct cFILE. */
	typedef struct cFILE cFILE;
	
	/*!\ingroup cconfig
	 * Free blocks storing strategy which determines whether should storage file be optimized for filesystem supporing sparce files or not.
	*/
	enum cSPARSE{
		SPARSE, //!< \brief Fill [free blocks](\ref cFBLOCK) with zero bytes, i.e. 0x00. Sparce optimization is enabled
		NOSPARSE,//!< \brief Sparce optimization is disabled. Free blocks are stored as is.
	};
	
	/*!\ingroup cconfig 
	 * Strategy of writing blocks (new or modified blocks).*/
	enum cFLASH_MODE{
		CDUMMY_MODE, //!<  Free blocks are ignored. Compressed block is written at the end of the storage file (archive). Fastest but not space efficient
		COPTIMIZED_MODE, //!< Space saving strategy. Compressed block is written instead of free block with minimum suitable size. Size of such free block's size is larger or equal to compressed block's size. Slowest one.
		CFIRST_SUITABLE_MODE,//!< A strategy between \ref CDUMMY_MODE and \ref COPTIMIZED_MODE. Compressed block is written instead of first found free block in storage file with larger or equal size.
	};
	/*!\ingroup cconfig 
	 * Property which determines whether user is permitted to perform operation of inserion into file (stream) or cutting file content.*/
	enum cALLOW_INSERTION{
		CENABLE, //!< \brief  Property is enabled.
		CDISABLE, //!< \brief Property is disabled.
	};

	/*!\ingroup cconfig 
	 * Strategy of resolving storage configuration conflicts. Determines an action if conflict occured. */
	enum cCFG_CONFLICT {
		FILE_CFG, //!< Prefer  storage configuration determined in storage file, i.e. ignore user configuration.
		RAISE_ERR,//!< Inform user that configuration conflict occured. Delegate choosing configuration to user.
	};
	
	/*!\ingroup cconfig 
	 * \brief Structrue representing user [cSTORAGE](\ref cSTORAGE) config.*/ 
	struct cCONFIG {
		/*! @name Storage Configuration
		 * \brief \ref cCONFIG properties for [storage](\ref cSTORAGE) behaviour configuration.
		 */
		/*!@{*/	
		size_t base_block_size;//!< \brief Base uncompressed block size.
		//Blocks cache size in blocks
		size_t cache_size;//!< \brief Blocks cache size in blocks.
						  /*!< if \ref cache_size is set to 0, then [blocks cache](\ref cCACHE) if disabled.*/
		double fragmentation_threshold;//!< \brief Treshold for space overusage by free blocks. 
									   //!
									   //! \ref fragmentation_threshold is obtained as all free blocks size divided by all compressed blocks size.
		double blocks_fragmentation_threshold;//!< \brief Treshold for blocks overusage by partially filled blocks.
											  //!
											  //! \ref blocks_fragmentation_threshold is  obtained as partial used blocks count divided by minimum needed fully filled blocks amount.
		// Enablement of torage optimization for filesystems supporting sparce files. 
		enum cSPARSE to_sparse;//!< \brief Enablement of torage optimization for filesystems supporting sparce files.
							   /*! [Free blocks](\ref cFBLOCK) storing strategy which determines whether should storage file be optimized for filesystem supporing sparce files or not. */
								
		// Strategy of writing blocks to storage. 
		enum cFLASH_MODE flash_method;//!< \brief  Strategy of writing blocks to storage.
		// Library behaviour if storage conflicts occur. 
		enum cCFG_CONFLICT on_conflict;//!< \brief  Library behaviour if storage conflicts occur.
		// Enablement of data insertion to file and data deletion from file
		enum cALLOW_INSERTION allow_insertion;//!< \brief  Enablement of data insertion to file and data deletion from file.
		/*!@}*/
		
		/*! @name Codec Configuration
		 * \brief \ref cCONFIG properties for compression/decompression codec configuration. */
		/*!@{*/
		char* codec_name;//!< \brief compression/decompression codec name.
						 /*!< \ref codec_name should be a valid [SquashCodec](https://quixdb.github.io/squash/api/c/group__SquashCodec.html) name  or a `custom` C-style string \n If a `"custom"` value is set then user is to provide an additional three functions:
						  *	- [compress](\ref compress)
						  *	- [uncompress](\ref uncompress)
						  *	- [get_max_compressed_size](\ref get_max_compressed_size)
						  * .
						  */
		//init with SquashStatus squash_codec_init(SquashCodec* codec)
		SquashCodec* squash_codec;//!< \brief Pointer to SquashCodec instance. 
								  /*!< This field is supposed to be initialized by `squash_codec_init` function.*/
								  //!< \warning This field is managed by means of inner library functions and should not be modified by user.
		//init with squash_oprion_init or squash_options_new
		SquashOptions* squash_codec_options;//!< \brief Compression/decompression options for `SquashCodec`.
											/*!< Represent a set of `key/value` pairs of options*/

		//compressed_lenght, compressed_buff, decompressed_lenght, decompressed_buff
		/*! \brief A user custom compression function.
		 * 
		 * \ref compress has a following signature `int compress(size_t* compressed_lenght, void* compressed_buff, size_t decompressed_lenght, const void* decompressed_buff)`. 
		 * \param [inout] compressed_lenght size of data after compression.
		 * \param [out] compressed_buf output buffer for stroing compressed data.
		 * \param [in] decompressed_lenght size of data for compression.
		 * \param [in] decompressed_buff input buffer storing data for compression.
		 * \return 0 on success and -1 otherwise.
		 */
		int (*compress)(size_t*, void*, size_t, const void*);
		//decompressed_leght, decompressed_buff, compressed_lenght, compressed_lenght
		/*! \brief A user custom decompression function.
		 *
		 * \ref uncompress has a following signature `int uncompress(size_t* decompressed_lenght, void* decompressed_buff, size_t compressed_lenght, const void* compressed_buff)`. 
		 * \param [inout] decompressed_lenght size of data after decompression.  
		 * \param [out] decompressed_buff output buffer storing a decompressed data.
		 * \param [in] compressed_lenght size of compressed data for decompression.
		 * \param [in] compressed_buff input buffer storing compressed data.
		 * \return 0 on success and -1 otherwise.
		 */
		int (*uncompress)(size_t*, void*, size_t, const void*);
		//decompressed_lenght
		/*! \brief Get the maximum buffer size necessary to store compressed data.
		 *
		 * \ref get_max_compressed_size has following signature `size_t get_max_compressed_size(size_t decompressed_lenght)`.
		 * \param decompressed_lenght Size of uncompressed data in bytes.
		 * \return The maximum size required to store a compressed buffer representing `decompressed_lenght` of uncompressed data.
		 */ 
		size_t (*get_max_compressed_size)(size_t);
		/*!@}*/
	};
	/*!\ingroup cconfig
	 * \brief typedef of struct cCONFIG.*/
	typedef struct cCONFIG cCONFIG;
	
	/*! \ingroup cconfig
	 *  \brief Generate a default configuration.
	 *	A list of default values:
	 *		- [base_block_size](\ref cCONFIG::base_block_size) is set to `16384`, i.e. 16Kb;
	 *		- [cache_size](\ref cCACHE) is set to `0`;
	 *		- [fragmentation_threshold](\ref cCONFIG::fragmentation_threshold) is set to `0`;
	 *		- [blocks_fragmentation_threshold](\ref cCONFIG::blocks_fragmentation_threshold) is set to `0`;
	 *		- [to_sparse](\ref cSPARSE) is set to \ref SPARSE;
	 *		- [flash_mehtod](\ref cFLASH_MODE) is set to \ref CDUMMY_MODE;
	 *		- [on_conflict](\ref cCFG_CONFLICT) is set to RAISE_ERR;
	 *		- [allow_insertion](\ref cALLOW_INSERTION) is set to \ref CDISABLE.
	 *	\return initialized pointer to \ref cCONFIG structure with default field values.
	 *	\warning \ref cCONFIG::codec_name is not set by default.
	 */
	cCONFIG* get_default_cfg();	
	/*! \ingroup cstorage 
	 * \brief Open a storage file.
	 *
	 * Initilizes cSTORAGE structure instance configured according to \ref cCFG_CONFLICT.
	 * \param [in] filename storage file name to open.
	 * \param [inout] cfg user [configiration](\ref cCONFIG) of storage behaviour.
	 * \return opened storage file, or \b NULL if error occured.
	 */ 
	cSTORAGE* copen(const char *filename, struct cCONFIG *cfg);
	
	/*! \ingroup cstorage 
	 * \brief Close storage file. 
	 *
	 * Frees cSTORAGE structure and closes assotioted with it file.
	 * \param [in] storage pointer to \ref cSTORAGE structure instance, which is being closed.
	 * \return 0 if succeeded and -1 otherwise
	 */
	int cclose(cSTORAGE* storage);
	
	/*! \ingroup cstorage 
	 * \brief Remove a file inside of storage file. 
	 * 
	 * Declares all file blocks as free blocks.
	 * \param storage pointer to \ref cSTORAGE structure instance, which contains file.
	 * \param filename  file name string pointer, a [file](\ref cFILE)  which is being deleted from [storage](\ref cSTORAGE).
	 * \return 0 if succeeded and -1 otherwise.
	 */
	int cremove(cSTORAGE* storage, const char* filename);
	
	/*!\ingroup cstorage
	 * \brief Changes  a file (stream) name  in storage.
	 *
	 * \param storage  pointer to \ref cSTORAGE structure instance containing a [file](\ref cFILE).
	 * \param old_filename file name string pointer to [file](\ref cFILE) which is being renamed.
	 * \param new_filename  string pointer containing a new filename.
	 * \return 0 if succeeded and -1 otherwise
	 */
	int crename(cSTORAGE* storage, const char* old_filename, const char* new_filename);

	/*!\ingroup cfile 
	 * \brief  Open a file (stream) in storage.
	 *
	 *  Initializes a \ref cFILE structure instance and assosiates with it a data stream in [storage](\ref cSTORAGE) file.
	 *  \param [in] storage  pointer to \ref cSTORAGE structure instance containing a [file](\ref cFILE).
	 *  \param [in] filename  file name string pointer of [file](\ref cFILE) to be opened.
	 *  \param [in] mode a string pointer, similar to `fopen` standard function mode. Can be one of following: 
	 *  	- `r`  open file for reading, stream is positioned at the begining of file.
	 *   	- `r+`  open file for reading and writing, stream is positioned at begining of file.
	 *   	- `w`  truncate file to zero lenght or create file for writing, stream is positioned at the begining of the file.
	 *   	- `w+`  open file for reading and writing. File is created if not exists, otherwise it is truncated to zero lenght. stream is positioned at the begining of the file.
	 *   	- `a`  open file for appending. File is created if not exists. The stream is positioned at the end of the file.
	 *   	- `a+`  open file for reading and appending. File is created if not exists. The stream is positioned at the end of the file.
	 *  \returns pointer to \ref cFILE structure instance and \b NULL otherwise.
	*/
	cFILE* cfopen(cSTORAGE* storage, const char* filename, const char* mode);

	
	/*!\ingroup cfile 
	 * \brief  Close file in a container.
	 *
	 * Frees \ref cFILE structure instance, calls \ref cfsync to flush cached blocks.
	 * \param [in] cfile  pointer to \ref cFILE structure instance to be closed.
	 * \return 0 if succeeded and -1 otherwise.
	 */
	int cfclose(cFILE* cfile);
	
	/*!\ingroup cfile_fpos 
	 * \brief File base position labels similar to standard stdio library.*/	
	enum cWHENCE{
		cSEEK_SET,//!< \brief  Begining of the [file](\ref cFILE).
		cSEEK_CUR, //!< \brief End of the [file](\ref cFILE).
		cSEEK_END, //!< \brief Current [file](\ref cFILE) position.
	};
	/*!\ingroup cfile_fpos
	 *  \brief typedef of enum cWHENCE.*/
	typedef enum cWHENCE cWHENCE;
	
	/*!\ingroup cfile_fpos 
	 * \brief  Change current file (stream) position.
	 *
	 *  Position is obtaned by adding offset to the position specified by [whence](\ref cWHENCE), i.e. current position, begining of the [file](\ref cFILE), end of the [file](\ref cFILE).
	 *   \param [in] cfile pointer to \ref cFILE structure instance.
	 *   \param [in] offset offset for obtaining new [file](\raf cFILE) position.
	 *   \param [in] whence base [file](\ref cFILE) position label.
	 *   \return 0 if succeeded and -1 otherwise.
	 */
	int cfseek(cFILE* cfile, long offset, cWHENCE whence);
	
	/*!\ingroup cfile_fpos
	 * \brief  Get current file (stream) position.
	 *
	 * \param [in] cfile  pointer to \ref cFILE structure instance.
	 * \return current file (stream) pointed by `cfile` or -1 on error.
	 */
	long int cftell(cFILE* cfile);

	/*!\ingroup cfile_fpos 
	 * \brief  Set file (stream) position to the begining of file (stream).
	 *
	 * \param [in] cfile pointer to \ref cFILE structure instance.
	 * \return This function has no return value.
	 */
	void crewind (cFILE* cfile);
	
	/*!\ingroup cfile 
	 * \brief  Syncronize changes from block's cache to a file (strem) in storage.
	 *
	 * \param  cfile  pointer to \ref cFILE structure instance.
	 * \return 0 if succeeded and -1 otherwise.
	 */
	int cfsync(cFILE* cfile);

	/*!\ingroup cfile
	 * \brief  Defragmentates file (stream) blocks. 
	 *
	 * Merges blocks resulting in lack of partially filled block.
	 * \param cfile  pointer to \ref cFILE structure instance.
	 * \return 0 if succeeded and -1 otherwise.
	 */
	int cfdefrag(cFILE* cfile);


	/*!\ingroup cfile_io
	 * \brief Read buffer from file (stream).
	 *
	 * Reads  `nmemb`  items of data, each `size` bytes long, from the stream pointed to by `stream`, storing them at the location given by `ptr`.
	 * \praram [out] ptr pointer to buffer for read data.
	 * \param [in] size size of item to read.
	 * \param [in] nmemb --- anount of item to be read.
	 * \param [in] cfile  pointer to \ref cFILE structure instance.
	 * \return amount of successfully read data items less or equal than `nmemb`.
	 */
	size_t cfread(void* ptr, const size_t size, const size_t nmemb, cFILE* cfile);
	
	/*!\ingroup cfile_io
	 * \brief  Write buffer to file (stream).
	 *
	 * Writes `nmemb` items of data, each `size` bytes long, to the stream pointed to by `stream`, obtaining them from the  location given by `ptr`.
	 * \param [in] ptr pointer to data to be written.
	 * \param [in] size size of item to be written.
	 * \param [in] nmemb amount of item to be writted.
	 * \param [in] cfile pointer to cFILE structure instance.
	 * \return amount of successfully written items less or equal than `nmemb`.
	 */
	size_t cfwrite(void* ptr, const size_t size, const size_t nmemb, cFILE* cfile);
	
	/*!\ingroup cfile_io
	 * \brief Delete data from file (stream).
	 *
	 * Deletes `nmemb` amount of bytes form file from starting with position obtained by adding `offset` to postion pointed by [whence](\ref cWHENCE) if \ref cALLOW_INSERTION is `ENABLED`.
	 * \param [in] cfile pointer to \ref cFILE structure instance.
	 * \param [in] whence base file (stream) position label.
	 * \param [in] offset offset for obtaining first deleted byte position.
	 * \param [in] nmemb amount of bytes to be deleted from file (stream).
	 * \return amount of successfully deleted bytes, less or equal than `nmemb`.
	 */
	size_t cfcut(cFILE* cfile, cWHENCE whence, long int offset, const size_t nmemb);
	
	/*!\ingroup cfile_io 
	 * \brief  Insert data to file (stream).
	 *
	 * Inserts nmemb amount of bytes from location pointed by ptr to file (stream) strting with position obtained by addning offset to whence if \ref cALLOW_INSERTION is `ENABLED`. If ptr is \b NULL-value then into file will be inserted `nmemb` of zero bytes (`\0`).
	 * \param [in] cfile  pointer to \ref cFILE structure instance.
	 * \param [in] whence  base file (stream) postition label.
	 * \param [in] offset  offset for obtaining first inserted byte postition.
	 * \param [in] nmemb  amount of bytes to be inserted to file (stream).
	 * \param [in] ptr  source buffer containing data for insertion. Must be at least `nmemb` bytes lenght.
	 * \return amount of successfully inserted bytes, less or equal than `nmemb`.
	 */
	size_t cfinsert(cFILE* cfile, cWHENCE whence, long int offset, const size_t nmemb, const void* ptr);


	/*!\ingroup cfile_io 
	 * \brief Formatted write to file (stream).
	 *
	 *  Writes an output under the control of a format string that specifies how subsequent arguments are converted into an output. Similar to standard function `fprintf`.\n Each format derective and conversion specification  results in fetching zero or more subsequent arguments. Each conversion specification is introduced by the character `%` and ends with a conversion specifier.  Syntax of a conversion specification is `%[$][flags][width][.precision][length modifier]conversion`. 
	 *   \param [in] cfile  pointer to \ref cFILE structure instance.
	 *   \param [in] format  pointer to format string containing derectives and conversion specifications
	 *   \param  ...  input arguments for conversion.
	 *   \return amount of successfully written bytes or -1 if error occured.
	 */
	int cfprintf(cFILE* cfile, const char* format, ...);
	
	/*!\ingroup cfile_io 
	 * \brief  Formatted write to file (stream).
	 *
	 *  Equivalent ot `cfprint` function exept that it is called with va list instead of variable number of arguments.\n Each format derective and conversion specification  results in fetching zero or more subsequent arguments. Each conversion specification is introduced by the character `%` and ends with a conversion specifier.  Syntax of a conversion specification is `%[$][flags][width][.precision][length modifier]conversion`. 
	 *  \param [in] cfile pointer to \ref cFILE structure instance.
	 *  \param [in] format pointer to format string containing derectives and conversion specifications. 
	 *  \param [in] ap  `va list` of argument for conversion.
	 *  \return amount of successfully written bytes or -1 if error occured.
	 */
	int cvfprintf(cFILE* cfile, const char* format, va_list ap);

	/*!\ingroup cfile_io
	 * \brief  Formatted read from file (stream).
	 *
	 *  Reads data from file according to format string that specifies how input is converted to subsequent arguments. Similar to standard function `fscanf`.\n Each format derective and conversion specification  results in fetching zero or more subsequent arguments. Each conversion specification is introduced by the character `%` and ends with a conversion specifier.  Syntax of a conversion specification is `%[$][flags][width][.precision][length modifier]conversion`. 
	 *  \param [in] cfile  pointer to \ref cFILE structure instance.
	 *  \param [in] format pointer to format string containing derectives and conversion specifications. 
	 *  \param ...  output arguments for conversion.
	 *  \return amount of successfully matched and assigned items, less or equal to amount of provided arguments or -1 if error occured.
	 */
	int cfscanf(cFILE* cfile, const char* format, ...);

	/*!\ingroup cfile_io
	 * \brief  Formatted read from file (stream).
	 *
	 * Equivalent to \ref cfscanf function exept that it is called with `va list` instead of variable number of arguments. \n Each format derective and conversion specification  results in fetching zero or more subsequent arguments. Each conversion specification is introduced by the character `%` and ends with a conversion specifier.  Syntax of a conversion specification is `%[$][flags][width][.precision][length modifier]conversion`. 
	 * \param [in] cfile pointer to \ref cFILE structure instance.
	 * \param format  pointer to format string containing derectives and conversion specifications.
	 * \param [out] ap `va list` of output arguments for conversion.
	 *  \return amount of successfully matched and assigned items, less or equal to amoun of provided arguments or -1 if error occured.
	 */
	int cvfscanf(cFILE* cfile, const char* format, va_list ap);
	
	/*!\ingroup cfile_io
	 * \brief  Read character from file (stream).
	 * \param [in] cfile pointer to \ref cFILE structure instance 
	 * \return read character as unsigned int or -1 if error occured.
	 */
	int cfgetc(cFILE* cfile);

	/*!\ingroup cfile_io
	 * \brief Read string from file (stream).
	 *
	 *  Reads no more than `size` characters from [file](\ref cFILE) and stores them in buffer pointed by `s`. Reading stops after end of file (stream) or new line. Newline is stored in `s`. `s` ends with terminator byte '\0'. 
	 *  \param [out] s  pointer to output string (buffer) where read characters will be stored.
	 *  \param [in] size  amount of characters to read.
	 *  \param [in] cfile  pointer to \ref cFILE structure instance.
	 *  \return `s` on success and \b NULL if error occured.
	 */
	char* cfgets(char* s, int size, cFILE* cfile);

	/*!\ingroup cfile_io
	 * \brief  Write character to a file (stream).
	 *
	 *  Writes a character `c` to [file](\ref cFILE) pointed by `cfile` casting it to unsigned character.
	 *  \param [in] c character to write.
	 *  \param [in] cfile pointer to \ref cFILE structure instance.
	 *  \returns: written unsigned character casted to int or -1 if error occured.
	 */
	int cfputc(int c, cFILE* cfile);

	/*!\ingroup cfile_io
	 * \brief Write string from a file (stream).
	 *
	 *  Writes a string pointed by `s` to a [file](\ref cFILE) pointed by `cfile` without terminating byte '"\0"'.
	 *  \param [in] s pointer to string to be written.
	 *  \param [in] cfile  pointer to \ref cFILE structure instance.
	 *  \return 0 on success or -1 if error occured.
	 */
	int cfputs(const char* s, cFILE* cfile);

	

#ifdef __cplusplus
}
#endif
#endif // _COMPRESSEDIO_H
