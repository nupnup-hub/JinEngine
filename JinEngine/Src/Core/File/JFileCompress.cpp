#include"JFileCompress.h"
#include <stdio.h>
#include <fast-lzma2.h> /* Assumes that libfast-lzma2 was installed using 'make install' */

using uint = unsigned int;
namespace JinEngine
{
	namespace Core
	{
		struct JCompressFile
		{
		public:
			FILE* in;
			FILE* out;
			//FILE* copied;
		public:
			FL2_CStream* cs;
			FL2_DStream* ds;
		};

		namespace
		{
			static std::string CreateDummy(const uint count)
			{
				std::string res;
				res.resize(count);
				for (uint i = 0; i < count; ++i)
					res[i] = ',';
				return res;
			}
			static bool StuffDecompressedData(_Inout_ JCompressInfo& info,
				JCompressFile& file,
				FL2_outBuffer& outBuff,
				size_t& fileInSize,		//buff -> file	read count
				size_t& buffOutSize,	//buff -> file	write count
				size_t& buffOutOffset,
				int& fileIndex,
				const bool toFile)
			{
				const size_t fileSize = info.fileInfo[fileIndex].fileSize;
				if (fileInSize + buffOutSize >= fileSize)
				{
					size_t inSize = fileSize - fileInSize;
					if (info.fileInfo[fileIndex].decomposeTarget)
					{
						if (toFile)
						{
							fwrite((char*)outBuff.dst + buffOutOffset, sizeof(char), inSize, file.out);
							fclose(file.out);
						}
						else
							info.fileInfo[fileIndex].stream.write((char*)outBuff.dst + buffOutOffset, inSize);
					}
					++fileIndex;
					buffOutSize -= inSize;
					buffOutOffset += inSize;
					fileInSize = 0;

					if (fileIndex < info.fileInfo.size())
					{
						if (toFile && info.fileInfo[fileIndex].decomposeTarget)
						{
							fopen_s(&file.out, info.fileInfo[fileIndex].path.c_str(), "wb");
							if (file.out == NULL)
							{
								info.errLog = info.compressPath + " Cannot open out file.\n";
								return false;
							}
						}
					}
				}
				else
				{
					if (info.fileInfo[fileIndex].decomposeTarget)
					{
						if (toFile)
							fwrite((char*)outBuff.dst + buffOutOffset, sizeof(char), buffOutSize, file.out);
						else
							info.fileInfo[fileIndex].stream.write((char*)outBuff.dst + buffOutOffset, buffOutSize);
					}
					fileInSize += buffOutSize;
					buffOutSize = 0;
				}
				return true;
			}
		}
		namespace LAZMA
		{
			static bool CreateFL2Cstreams(_Inout_ JCompressInfo& info, JCompressFile& file)
			{
				file.cs = FL2_createCStreamMt(2, 0);
				if (file.cs == NULL)
				{
					info.errLog = "Cannot allocate compression context.\n";
					return false;
				}
				else
					return true;
			}
			static bool CreateFL2Dstreams(_Inout_ JCompressInfo& info, JCompressFile& file)
			{
				file.ds = FL2_createDStreamMt(2);
				if (file.ds == NULL)
				{
					info.errLog = "Cannot allocate decompression context.\n";
					return false;
				}
				else
					return true;
			}
			static bool InitFL2CStreams(_Inout_ JCompressInfo& info, JCompressFile& file)
			{
				size_t res = FL2_initCStream(file.cs, info.preset);
				if (FL2_isError(res))
				{
					info.errLog = FL2_getErrorName(res);
					return false;
				}
				else
					return true;
			}
			static bool InitFL2DStreams(_Inout_ JCompressInfo& info, JCompressFile& file)
			{
				size_t res = FL2_initDStream(file.ds);
				if (FL2_isError(res))
				{
					info.errLog = FL2_getErrorName(res);
					return false;
				}
				else
					return true;
			}
			static bool Compress(_Inout_ JCompressInfo& info, JCompressFile& file)
			{
				unsigned char inBuf[8 * 1024];
				unsigned char outBuf[4 * 1024];
				FL2_inBuffer inBufInfo = { inBuf, sizeof(inBuf), sizeof(inBuf) };
				FL2_outBuffer outBufInfo = { outBuf, sizeof(outBuf), 0 };
				size_t res = 0;
				size_t inSize = 0;
				size_t outSize = 0;

				InitFL2CStreams(info, file);
				for (uint i = 0; i < info.fileInfo.size(); ++i)
				{
					fopen_s(&file.in, info.fileInfo[i].path.c_str(), "rb");
					if (file.in == NULL)
					{
						info.errLog = info.fileInfo[i].path + " Cannot open in file.\n";
						return false;
					}

					info.fileInfo[i].fileSize = 0;
					do
					{
						if (inBufInfo.pos == inBufInfo.size)
						{
							inBufInfo.size = fread(inBuf, 1, sizeof(inBuf), file.in);
							inSize += inBufInfo.size;
							info.fileInfo[i].fileSize += inBufInfo.size;
							inBufInfo.pos = 0;
						}
						res = FL2_compressStream(file.cs, &outBufInfo, &inBufInfo);
						if (FL2_isError(res))
						{
							info.errLog = FL2_getErrorName(res);
							fclose(file.in);
							return false;
						}

						fwrite(outBufInfo.dst, 1, outBufInfo.pos, file.out);
						outSize += outBufInfo.pos;
						outBufInfo.pos = 0;
					} while (inBufInfo.size == sizeof(inBuf));
					fclose(file.in);
				}
				do
				{
					res = FL2_endStream(file.cs, &outBufInfo);
					if (FL2_isError(res))
					{
						info.errLog = FL2_getErrorName(res);
						fclose(file.in);
						return false;
					}

					fwrite(outBufInfo.dst, 1, outBufInfo.pos, file.out);
					outSize += outBufInfo.pos;
					outBufInfo.pos = 0;
				} while (res);
				return true;
			}
			static bool Decompress(_Inout_ JCompressInfo& info, JCompressFile& file, const bool toFile)
			{
				unsigned char inBuf[4 * 1024];
				unsigned char outBuf[8 * 1024];
				FL2_inBuffer inBufInfo = { inBuf, sizeof(inBuf), sizeof(inBuf) };
				FL2_outBuffer outBufInfo = { outBuf, sizeof(outBuf), 0 };
				size_t res;
				size_t inSize = 0;
				size_t outSize = 0;

				size_t fileInSize = 0;
				int fileIndex = 0;
				if (toFile && info.fileInfo[fileIndex].decomposeTarget)
				{
					fopen_s(&file.out, info.fileInfo[fileIndex].path.c_str(), "wb");
					if (file.out == NULL)
					{
						info.errLog = info.compressPath + " Cannot open out file.\n";
						return false;
					}
				}

				InitFL2DStreams(info, file);
				do {
					if (inBufInfo.pos == inBufInfo.size)
					{
						inBufInfo.size = fread(inBuf, 1, sizeof(inBuf), file.in);
						inSize += inBufInfo.size;
						inBufInfo.pos = 0;
					}

					res = FL2_decompressStream(file.ds, &outBufInfo, &inBufInfo);
					if (FL2_isError(res))
					{
						info.errLog = FL2_getErrorName(res);
						if (toFile)
							fclose(file.out);
						return false;
					}
					/* Discard the output. XXhash will verify the integrity. */
					if (outBufInfo.pos > 0)
					{
						size_t outPos = outBufInfo.pos;
						size_t outOffset = 0;
						while (outPos != 0 && StuffDecompressedData(info, file, outBufInfo, fileInSize, outPos, outOffset, fileIndex, toFile));
					}
					outSize += outBufInfo.pos;
					outBufInfo.pos = 0;

				} while (res && inBufInfo.size);

				return true;
			}
			static bool CompressProcess(_Inout_ JCompressInfo& info)
			{
				JCompressFile file;
				if (!CreateFL2Cstreams(info, file))
					return false;

				fopen_s(&file.out, info.compressPath.c_str(), "w+b");
				if (file.out == NULL)
				{
					info.errLog = info.compressPath + " Cannot open out file.\n";
					return false;
				}

				const bool res = Compress(info, file);
				fclose(file.out);
				FL2_freeCStream(file.cs);
				return res;
			}
			static bool DecompressProcess(_Inout_ JCompressInfo& info, const bool toFile)
			{
				JCompressFile file;
				if (!CreateFL2Dstreams(info, file))
					return false;

				fopen_s(&file.in, info.compressPath.c_str(), "rb");
				if (file.in == NULL)
				{
					info.errLog = "Cannot open input file.\n";
					return false;
				}

				const bool res = Decompress(info, file, toFile);
				fclose(file.in);
				FL2_freeDStream(file.ds);
				return res;
			}
		}

		bool JFileCompression::Compress(_Inout_ JCompressInfo& info)
		{
			if (info.fileInfo.size() == 0)
				return false;

			return LAZMA::CompressProcess(info);
		}
		bool JFileCompression::DecompressToFile(_Inout_ JCompressInfo& info)
		{
			if (info.fileInfo.size() == 0)
				return false;

			return LAZMA::DecompressProcess(info, true);
		}
		bool JFileCompression::DecompressToStream(_Inout_ JCompressInfo& info)
		{
			if (info.fileInfo.size() == 0)
				return false;

			return LAZMA::DecompressProcess(info, false);
		}
	}
}