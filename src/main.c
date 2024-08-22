#include "global.h"
#include <unistd.h>
#include <windows.h>

#define HEX04_SRAM_SIZE (0x700000/8) //7*1024*1024/8=917504Bytes

void execute_command(const char *format, ...) {
    char command[1024];  // 定义一个足够大的字符数组来保存命令字符串

    // 使用 va_list 和 va_start 宏来处理可变参数
    va_list args;
    va_start(args, format);

    // 使用 vsprintf 函数将可变参数格式化为命令字符串
    vsprintf(command, format, args);

    // 结束可变参数的处理
    va_end(args);

    // 调用 system 函数执行命令
    int ret = system(command);

}

int main(int argc, char *argv[])
{
	int opt;
	uint16_t width = 720, height = 1600, line_num = 20, width_aligned = 0;
	uint32_t encode_rate = 0, encode_size = 0;

    while ((opt = getopt(argc, argv, "w:h:l:r:")) != -1) {
        switch (opt) {
            case 'w':
                width = atoi(optarg);
                break;
            case 'h':
                height = atoi(optarg);
                break;
			case 'l':
                line_num = atoi(optarg);
                break;
            case 'r':
                encode_rate = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -w pic width -h pic heigh -l encode line num -r encode rate\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

	if(!width || !height || !line_num)
	{
		fprintf(stderr, "Usage: %s -w pic width -h pic heigh -l encode line num -r encode rate\n", argv[0]);
	    return 0;
	}



	encode_size = (HEX04_SRAM_SIZE/64) / ((line_num+height-1)/line_num);
	encode_size = encode_size *64;
	encode_size = !encode_rate ? encode_size : width*line_num*2/encode_rate;

	//The width must be 64 bytes aligned
  	width_aligned = 64*((width+63)/64);

    uint32_t Ysize = width_aligned * line_num;
	uint32_t UVsize = width_aligned * line_num / 2;
	

    // Allocate memory for U and V planes
	uint8_t *yPlane = (uint8_t *)malloc(Ysize);
    uint8_t *uPlane = (uint8_t *)malloc(UVsize);
    uint8_t *vPlane = (uint8_t *)malloc(UVsize);

	printf("width=%d, height=%d, line num=%d, block_size=%d\n", width, height, line_num, encode_size);


	// memset(yPlane, 0, Ysize);
	// for(uint32_t i=0;i<line_num;i++)
	// {
	// 	memset(uPlane + i * width_aligned/2, 128, width/2);
	// 	//memset(uPlane + width/2, 0, (width_aligned-width)/2);
	// }
	// memcpy(vPlane, uPlane, UVsize);

	memset(yPlane, 0, Ysize);
	memset(uPlane, 128, UVsize);
	memset(vPlane, 128, UVsize);

    // 创建输出文件以写入结果
    FILE * yuvFile = fopen("temp.yuv", "wb");
    if (yuvFile == NULL) {
        printf("fail to open output file\n");
        return 1;
    }

	fwrite(yPlane, 1, Ysize, yuvFile);
	fwrite(uPlane, 1, UVsize, yuvFile);
	fwrite(vPlane, 1, UVsize, yuvFile);
	fflush(yuvFile);
	fclose(yuvFile);

 	execute_command(".\\jxs_encoder.exe -c \"profile=Main444.12;size=%d;slh=%d; l=unrestricted;s=unrestricted;fs=1;gains=8, 6, 6, 7, 5, 5, 7, 4, 4, 6, 4, 4, 5, 3, 3, 3, 1, 1, 3, 1, 1, 2, 0, 0;priorities=12, 15, 16, 8, 10, 11, 21, 1, 0, 9, 14, 13, 17, 18, 19, 3, 4, 5, 2, 6, 7, 20, 22, 23\" -w %d -h %d  temp.yuv  temp.jxs", encode_size, line_num, width_aligned, line_num);
	execute_command(".\\convertEndian.exe -n 8  temp.jxs temp_LE.jxs");
	//execute_command(".\\convertEndian.exe -n 8  chip_dump_black.bin temp_LE.jxs");

    // 创建输出文件以写入结果
    FILE * jxsFile = fopen("temp_LE.jxs", "rb");
    if (jxsFile == NULL) {
        printf("fail to open input file\n");
        return 1;
    }

    FILE * black_jxs = fopen("framebuff_init_data.h", "w");
    if (black_jxs == NULL) {
        printf("fail to open output file\n");
        return 1;
    }

	uint8_t *jxs_buff = (uint8_t *)malloc(encode_size);
    uint8_t *encode_buff = (uint8_t *)malloc(encode_size);
	uint32_t encode_output_size = 0;

	fread(jxs_buff, 1, encode_size, jxsFile);
	fclose(jxsFile);
	encode_output_size = compressArray(jxs_buff, encode_size, encode_buff);

	fprintf(black_jxs, "uint8_t __attribute__((section(\".code\")))  framebuff_init_data[%d] = {", encode_output_size + 9);
	fprintf(black_jxs, "0x%02x, 0x%02x, 0x%02x, 0x%02x, ", encode_output_size & 0xff, (encode_output_size & 0xff00) >> 8, (encode_output_size & 0xff0000) >> 16, (encode_output_size & 0xff0000) >> 24);
	fprintf(black_jxs, "0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,", width&0xff, (width&0xff00)>>8, height&0xff, (height&0xff00)>>8, line_num);
	for(uint16_t i=0; i < encode_output_size; i++) {
		if((i%16) == 0)
		{
			fprintf(black_jxs, "\n");
		}
		fprintf(black_jxs, "0x%02x, ", encode_buff[i]);
	}
	fprintf(black_jxs, "\n};");

	fflush(black_jxs);
	fclose(black_jxs);

    return 0;
}