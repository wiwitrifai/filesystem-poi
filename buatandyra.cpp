void setNextBlock(ptr_block position, ptr_block next){
	filesys.NextBlock[position] = next;
	writeAllocTable(position);
}

ptr_block allocateBlock(){
	ptr_block result = filesys.firstEmpty;
	setNextBlock(result, END_BLOCK);
	while (filesys.NextBlock[firstEmpty] != 0x0000) {
		filsys.firstEmpty++;
	}
	filesys.Unused--;
	writeVolumeInfo();
	return result;
}

void freeBlock(ptr_block position){

	if (position == EMPTY_BLOCK) {
		return 0;
	}
		while (position != END_BLOCK) {
		ptr_block temp = filesys.NextBlock[position];
		setNextBlock(position, EMPTY_BLOCK);
		position = temp;
		(filesys.Unsed)--;
	}
	writeVolumeInfo();
}

int readBlock(ptr_block position, char *buffer, int size, int offset = 0){
	/* kalau sudah di END_BLOCK, return */
	if (position == END_BLOCK) {
		return 0;
	}
	/* kalau offset >= BLOCK_SIZE */
	if (offset >= BLOCK_SIZE) {
		return readBlock(filesys.NextBlock[position], buffer, size, offset - BLOCK_SIZE);
	}
	
	stream.seekg(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	/* cuma bisa baca sampai sebesar block size */
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	stream.read(buffer, size_now);
	
	/* kalau size > block size, lanjutkan di nextBlock */
	if (offset + size > BLOCK_SIZE) {
		return size_now + readBlock(stream.NextBlock[position], buffer + BLOCK_SIZE, offset + size - BLOCK_SIZE);
	}
	return size_now;
}

int writeBlock(ptr_block position, const char *buffer, int size, int offset = 0){
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
		return size_now + writeBlock(filesys.NextBlock[position], buffer + BLOCK_SIZE, offset + size - BLOCK_SIZE);
	}
	return size_now;
}