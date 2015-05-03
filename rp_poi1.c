/* rp_poi.c */

#include "rp_poi.h"

void createFilesystem(const char* path) {
	newFile = fopen(path, "w");
	
	/* INIT_VOLUME_INFO */

	fwrite("poi!", sizeof(char), 1, newFile);
	// Set default volume name with "POI!"
	strncpy(filesys.VolumeName, "POI!", 31);
	filesys.VolumeName[31] = '\0';

	// Total filesystem Capacity
	filesys.Capacity = N_BLOCK;

	// Unused block
	filesys.Unused = N_BLOCK-1;

	// First free block index
	filesys.FirstEmpty = 1;
	
	// path
	strncpy(filesys.path, path, 31);
	filesys.path[31] = '\0';

	/* INIT_ALLOCATION_TABLE */
	fwrite('\0', sizeof(short), sizeof())

	// block tidak mempunyai
	filesys.NextBlock[0] = 0xff;
	int i;
	for (i=1; i < N_BLOCK; i++) {
		filesys.NextBlock[0] = 0;
	}
	fwrite(filesys, sizeof(filesys), 1, newFile);
	fwrite("!iop", sizeof(char), 428, newFile);
	fclose(newFile);
}


void loadFilesystem(const char* path) {
	stream = fopen(path, "wr");

	char buffer[BLOCK_SIZE];

	fread(buffer, sizeof(char), sizeof(char), stream);
	/* LOAD_VOLUME_INFO */
	if (strcmp(buffer,"poi!") != 0) {
		fclose(stream);
		printf("File bukan ektensi poi!");
	}
	fread(buffer, sizeof(char), 32, stream);
	strncpy(filesys.VolumeName, buffer, 31);
	filesys.VolumeName[31] = '\0';

	// read file_system Capacity
	fread(buffer, sizeof(char), sizeof(char), stream);
	filesys.Capacity = buffer;
	
	// read unused block
	fread(buffer, sizeof(char), sizeof(char), stream);
	filesys.Unused = buffer;

	// read first free block index
	filesys.FirstEmpty = buffer;

	// path
	strncpy(filesys.path, path, 31);
	filesys.path[31] = '\0';

	/* LOAD_ALLOCATION_TABLE */
	char buffer[4];

	for (int i=0; i < N_BLOCK; i++){
		fread(buffer, sizeof(char), sizeof(4), newFile);
		strcpy(filesys.NextBlock, buffer);
	}
}

void writeVolumeInfo() {
	fseek(stream, 0, SEEK_SET);

	fwrite("poi!", sizeof(char), 1, stream);

	fwrite(filesys.VolumeName, sizeof(char), stream);

	// Total filesystem Capacity
	fwrite(filesys.Capacity, sizeof(char), 1, stream);

	// Unused block
	fwrite(filesys.Unused, sizeof(char), 1, stream);	
	filesys.Unused = N_BLOCK-1;

	// First free block index
	fwrite(filesys.FirstEmpty, sizeof(char), 1, stream);
	
	// path
	fwrite(filesys.path, sizeof(char), 1, stream);

	fwrite('\0', sizeof(short), sizeof())
}

void writeAllocTable(ptr_block position) {
	fwrite('\0', sizeof(short), sizeof())
	// block tidak mempunyai
	filesys.NextBlock[0] = 0xff;
	fwrite(filesys.NextBlock[0], sizeof(char), 1, stream);
	for (int i=1; i < N_BLOCK; i++) {
		fwrite(filesys.NextBlock[i], sizeof(char), 1, stream);
	}
	fwrite(filesys, sizeof(filesys), 1, stream);
	fwrite("!iop", sizeof(char), 428, stream);	
}


void setNextBlock(ptr_block position, ptr_block next){
	filesys.NextBlock[position] = next;
	writeAllocaTable(position);
}

ptr_block allocateBlock(){
	ptr_block result = filesys.FirstEmpty;
	setNextBlock(result, END_BLOCK);
	while (filesys.NextBlock[filesys.FirstEmpty] != 0x0000) {
		filesys.FirstEmpty++;
	}
	filesys.Unused--;
	writeVolumeInfo();
	return result;
}

void freeBlock(ptr_block position){

	if (position == EMPTY_BLOCK) {
		return;
	}
		while (position != END_BLOCK) {
		ptr_block temp = filesys.NextBlock[position];
		setNextBlock(position, EMPTY_BLOCK);
		position = temp;
		(filesys.Unused)--;
	}
	writeVolumeInfo();
}

int readBlock(ptr_block position, char *buffer, int size, int offset){
	/* kalau sudah di END_BLOCK, return */
	if (position == END_BLOCK) {
		return 0;
	}
	/* kalau offset >= BLOCK_SIZE */
	if (offset >= BLOCK_SIZE) {
		return readBlock(filesys.NextBlock[position], buffer, size, offset - BLOCK_SIZE);
	}
	
	fseek(stream,BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	/* cuma bisa baca sampai sebesar block size */
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	fread(buffer, size_now);
	
	/* kalau size > block size, lanjutkan di nextBlock */
	if (offset + size > BLOCK_SIZE) {
		return size_now + readBlock(filesys.NextBlock[position], buffer + BLOCK_SIZE, offset + size - BLOCK_SIZE, 0);
	}
	return size_now;
}

int writeBlock(ptr_block position, const char *buffer, int size, int offset){
	 /* ketika posisi blok ada di END_BLOCK, tidak dapat menulis di blok */
	if (position == END_BLOCK) {
		return 0;
	}
	/* offset lebih dari BLOCK_SIZE */
	if (offset >= BLOCK_SIZE) {
		if(filesys.NextBlock[position] == END_BLOCK){
			setNextBlock(position, allocateBlock());
		}
		return writeBlock(filesys.NextBlock[position], buffer, size, offset);
	}
	stream.seekp(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	stream.write(buffer, size_now);
	
	/* kalau size > block size, lanjutkan di nextBlock */
	if (offset + size > BLOCK_SIZE) {
		/* kalau nextBlock tidak ada, alokasikan */
		if (filesys.NextBlock[position] == END_BLOCK) {
			setNextBlock(position, allocateBlock());
		}
		return size_now + writeBlock(filesys.NextBlock[position], buffer + BLOCK_SIZE, offset + size - BLOCK_SIZE,0);
	}
	return size_now;
}