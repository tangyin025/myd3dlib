
#include <crtdbg.h>
#include <string.h>
#include <stdio.h>
#include <share.h>
#include <zzip/lib.h>

// ------------------------------------------------------------------------------------------
// main
// ------------------------------------------------------------------------------------------

int main(int argc, char ** argv)
{
	// 设置crtdbg监视内存泄漏
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// 用以测试的ZIP文件
	const char * name = "test2.zip";

	// 分析输入命令
	if(argc > 1 && argv[1] != NULL)
	{
		if(!strcmp(argv[1], "--help"))
		{
			printf("zziptest [testfile]\n - selftest defaults to 'test.zip'");
			return 0;
		}
		else if(!strcmp(argv[1], "--version"))
		{
			printf(__FILE__ " version " ZZIP_PACKAGE " " ZZIP_VERSION " \n");
			return 0;
		}
		else
		{
			name = argv[1];
			argv++;
			argc--;
		}
	}

	// 打开指定的输入文件
	printf("Opening zip file `%s'... ", name);
	int fd;
	errno_t open_err = _sopen_s(&fd, name, O_RDONLY | O_BINARY, _SH_DENYWR, 0);
	if(0 != open_err)
	{
		printf("\n");
		perror("could not open input file");
		return 0;
	}

	// 从打开的文件转换为ZIP目录
	zzip_error_t rv;
	ZZIP_DIR * dir = zzip_dir_fdopen(fd, &rv);
	if(!dir)
	{
		printf("\n");
		printf("zzip_dir_fdopen error %d. \n", rv);
		return 0;
	}
	printf("OK. \n");

	// 检测ZIP文件的目录结构
	printf("{check ... \n");
	struct zzip_dir_hdr * hdr = dir->hdr0;
	if(NULL == hdr)
	{
		printf("could not find first header in dir_hdr \n");
	}
	else
	{
		while(1)
		{
			// 读取文件信息
			printf("compression method: %d ", hdr->d_compr);
			switch(hdr->d_compr)
			{
			case 0:
				printf("(stored)");
				break;
			case 8:
				printf("(deflated)");
				break;
			default:
				printf("(unknown)");
				break;
			}
			printf(" \n");
			printf("crc32: %x \n", hdr->d_crc32);
			printf("compressed size: %d \n", hdr->d_csize);
			printf("uncompressed size: %d \n", hdr->d_usize);
			printf("offset of file in archive: %d \n", hdr->d_off);
			printf("filename: %s \n", hdr->d_name);

			if(0 == hdr->d_reclen)
			{
				break;
			}

			// 定位到下一个文件
			char * tmp = (char *)hdr;
			tmp += hdr->d_reclen;
			hdr = (zzip_dir_hdr *)tmp;
			printf("\n");
		}
	}
	printf("} \n");

	// 检测ZIP文件的目录结构
	printf("\n");
	printf("{contents ... \n");
	for(int i = 0; i < 2; i++)
	{
		ZZIP_DIRENT * ent;
		while(ent = zzip_readdir(dir))
		{
			printf("name \"%s\", compr %d, size %d, ratio %2d\n",
				ent->d_name,
				ent->d_compr,
				ent->st_size,
				100 - (ent->d_csize | 1) * 100 / (ent->st_size | 1));
		}
		printf("%d. time ---------------\n", i + 1);
		zzip_rewinddir(dir);
	}
	printf("} \n");

	// 读取指定文件并输出
	name = argv[1] ? argv[1] : "README";
	printf("Opening file '%s' in zip archive ... ", name);
	ZZIP_FILE * fp = zzip_file_open(dir, name, ZZIP_CASEINSENSITIVE);
	if(!fp)
	{
		printf("\n");
		printf("zzip_file_open error %d: %s \n", zzip_error(dir), zzip_strerror_of(dir));
	}
	else
	{
		printf("OK. \n");
		printf("Contents of the file: \n");

		// 从文件中读取内容
		int i;
		char buf[17];
		while((i = zzip_file_read(fp, buf, sizeof(buf) - 1)) > 0)
		{
			buf[i] = '\0';
			printf("%s", buf);
		}
		if(i < 0)
		{
			printf("zzip_file_read error %d: %s \n", zzip_error(dir), zzip_strerror_of(dir));
		}
	}

	// 关闭文件
	zzip_file_close(fp);

	// 关闭目录（不再需要_close(fd)）
	zzip_dir_close(dir);

	return 0;
}
