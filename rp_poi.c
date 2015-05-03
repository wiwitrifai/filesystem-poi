/* rp_poi.c */

#include "rp_poi.h"

extern FILE * stream;
extern poi_file filesys;

void createFilesystem(const char* path) {
	FILE * newFile = fopen(path, "w");
	/* INIT_VOLUME_INFO */

	fwrite("poi!", sizeof(char), 4, newFile);
	// Set default volume name with "POI!"
	strncpy(filesys.VolumeName, "POI!", 31);
	filesys.VolumeName[4] = '\0';

	// Total filesystem Capacity
	filesys.Capacity = N_BLOCK;

	// Unused block
	filesys.Unused = N_BLOCK-1;

	// First free block index
	filesys.FirstEmpty = 1;

	// Entri directory root
	strcpy(filesys.Root.Name, "/");
	filesys.Root.Atribut = 0x0F;
	setCurrentDateTime(&filesys.Root);
	filesys.Root.IndexFirst = 0x01;
	filesys.Root.Size = BLOCK_SIZE;

	fwrite((char*)&filesys, sizeof(char), 76, newFile);
	char c = '\0';
	int i ;
	for(i = 0; i< 428; i++)
		fwrite(&c, sizeof(char), 1, newFile);
	fwrite("!iop", sizeof(char), 4, newFile);


	/* INIT_ALLOCATION_TABLE */

	// block root tidak mempunyai next
	filesys.NextBlock[0] = 0xffff;
	for (i=1; i < N_BLOCK; i++) {
		filesys.NextBlock[i] = 0x0000;
	}
	fwrite((char *)filesys.NextBlock, sizeof(char), N_BLOCK*2, newFile);

	/* INIT DATA POOL */
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	for(i = 0; i<N_BLOCK; i++) 
		fwrite(buffer, sizeof(char), BLOCK_SIZE, newFile);
	fclose(newFile);
}


void loadFilesystem(const char* path) {
	int i;
 	stream = fopen(path, "r+");

 	char buffer[BLOCK_SIZE];

	fread(buffer, sizeof(char), 4, stream);
	buffer[4] = '\0';

	/* LOAD_VOLUME_INFO */
	if (strcmp(buffer,"poi!") != 0) {
		fclose(stream);
		printf("File bukan ektensi poi!\n");
		system("EXIT");
	}
	fread(buffer, sizeof(char), 32, stream);
	strncpy(filesys.VolumeName, buffer, 32);
	filesys.VolumeName[31] = '\0';

	// read file_system Capacity
	fread(&filesys.Capacity, sizeof(int), 1, stream);
	
	// read unused block
	fread(&filesys.Unused, sizeof(int), 1, stream);

	// read first free block index
	fread(&filesys.FirstEmpty, sizeof(int), 1, stream);

	// path
	readEntryBlock(&filesys.Root, ftell(stream));

	/* LOAD_ALLOCATION_TABLE */
	fseek(stream, BLOCK_SIZE, SEEK_SET);
	for (i=0; i < N_BLOCK; i++){
		fread(filesys.NextBlock+i, sizeof(unsigned short), 1, stream);
	}
}

void writeVolumeInfo() {
	fseek(stream, 0x00, SEEK_SET);
	
	/* buffer untuk menulis ke file */
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	/* Magic string "CCFS" */
	memcpy(buffer + 0x00, "poi!", 4);
	
	/* Nama volume */
	memcpy(buffer + 0x04, filesys.VolumeName, strlen(filesys.VolumeName));
	
	/* Kapasitas filesystem, dalam little endian */
	memcpy(buffer + 0x24, (char*)&filesys.Capacity, 4);
	
	/* Jumlah blok yang belum terpakai, dalam little endian */
	memcpy(buffer + 0x28, (char*)&filesys.Unused, 4);
	
	/* Indeks blok pertama yang bebas, dalam little endian */
	memcpy(buffer + 0x2C, (char*)&filesys.FirstEmpty, 4);
	
	/* String "SFCC" */
	memcpy(buffer + 0x1FC, "!iop", 4);
	
	fwrite((char *)buffer, sizeof(char), BLOCK_SIZE, stream);
}

void writeAllocTable(ptr_block position) {
	fseek(stream, BLOCK_SIZE + sizeof(ptr_block)*position, SEEK_SET);
	fwrite((char*)&filesys.NextBlock[position], sizeof(short), 1, stream);
}


void setNextBlock(ptr_block position, ptr_block next){
 	filesys.NextBlock[position] = next;
 	writeAllocTable(position);
}

ptr_block allocateBlock(){
	ptr_block result = filesys.FirstEmpty;
	setNextBlock(result, END_BLOCK);
	while (filesys.FirstEmpty < N_BLOCK && filesys.NextBlock[filesys.FirstEmpty] != EMPTY_BLOCK) {
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
		(filesys.Unused)++;
	}
	writeVolumeInfo();
}
/** membaca isi block sebesar size kemudian menaruh hasilnya pada buffer */
int readBlock(ptr_block position, char *buffer, int size, int offset){
	/* kalau sudah di END_BLOCK, return */
	if (position == END_BLOCK) {
		return 0;
	}
	/* kalau offset >= BLOCK_SIZE */
	if (offset >= BLOCK_SIZE) {
		return readBlock(filesys.NextBlock[position], buffer, size, offset - BLOCK_SIZE);
	}

	fseek(stream, BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset, SEEK_SET);
	int size_now = size;
	/* cuma bisa baca sampai sebesar block size */
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	fread(buffer, sizeof(char), size_now, stream);

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
		return writeBlock(filesys.NextBlock[position], buffer, size, offset - BLOCK_SIZE);
	}
	fseek(stream, BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset, SEEK_SET);
	int size_now = size;
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	fwrite(buffer, sizeof(char), size_now, stream);
	
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

void readEntryBlock(entry_block * eb, int address) {
	fseek(stream, address, SEEK_SET);
	fread((*eb).Name, sizeof(char), 21, stream); 		// Name
	fread(&(*eb).Atribut, sizeof(char), 1, stream); 		// Atribut
	fread((*eb).Time, sizeof(char), 2, stream);		// Time
	fread((*eb).Date, sizeof(char), 2, stream);		// Date
	fread(&(*eb).IndexFirst, sizeof(unsigned short), 1, stream); // IndexFirst
	fread(&(*eb).Size, sizeof(int), 1, stream);			// Size
}
entry_block createEntryBlockEmpty() {
	entry_block eb;
	memset(&eb, 0, sizeof(eb));
	return eb;
}
/** entry block from location */
entry_block createEntryBlock (ptr_block position, unsigned char offset) {
	entry_block this;
	this.Position = position;
	this.Offset = offset;
	/* baca dari data pool */
	readEntryBlock(&this, BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset * ENTRY_SIZE);
	return this;
}
	/** Mendapatkan Entry berikutnya */
entry_block nextEntry(entry_block * eb) {
	if ((*eb).Offset < 15) {
		return createEntryBlock((*eb).Position, (*eb).Offset + 1);
	}
	else {
		return createEntryBlock(filesys.NextBlock[(*eb).Position], 0);
	}
}
/** Mendapatkan Entry dari path */
entry_block getEntry(entry_block * eb, const char *path) {
	/* mendapatkan direktori teratas */
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	char topDirectory[21];
	strncpy(topDirectory, path + 1, endstr - 1);
	topDirectory[endstr - 1] = '\0';
	/* mencari entri dengan nama topDirectory */
	while ( strcmp((*eb).Name, topDirectory) && (*eb).Position != END_BLOCK) {
		(*eb) = nextEntry(eb);
	}
	/* kalau tidak ketemu, return Entry kosong */
	if (isEmpty(eb)) {
		return createEntryBlockEmpty();
	}
	/* kalau ketemu, */
	else {
		if (endstr == strlen(path)) {
			return *eb;
		}
		else {
			/* cek apakah direktori atau bukan */
			if ((*eb).Atribut & 0x8) {
				ptr_block index;
				index = (*eb).IndexFirst;
				entry_block next = createEntryBlock(index, 0);
				return getEntry(&next, path + endstr);
			}
			else {
				return createEntryBlockEmpty();
			}
		}
	}
}
/** Mendapatkan Entry dari path */
entry_block getNewEntry(entry_block * eb, const char *path) {
	/* mendapatkan direktori teratas */
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	char topDirectory[21];
	strncpy(topDirectory, path + 1, endstr - 1);
	topDirectory[endstr-1] = '\0';
	/* mencari entri dengan nama topDirectory */
	entry_block entry = createEntryBlock((*eb).Position, (*eb).Offset);
	while (strcmp((*eb).Name, topDirectory) && (*eb).Position != END_BLOCK) {
		(*eb) = nextEntry(eb);
	}
	/* kalau tidak ketemu, buat entry baru */
	if (isEmpty(eb)) {
		while (!isEmpty(&entry)) {
			if (nextEntry(&entry).Position == END_BLOCK) {
				entry = createEntryBlock(allocateBlock(), 0);
			}
			else {
				entry = nextEntry(&entry);
			}
		}
		/* beri atribut pada entry */
		strcpy(entry.Name, topDirectory);
		entry.Atribut = 0xF;
		entry.IndexFirst = allocateBlock();
		entry.Size = BLOCK_SIZE;
		entry.Time[0] = 0;
		entry.Time[1] = 0;
		entry.Date[0] = 0;
		entry.Date[1] = 0;
		writeEntryBlock(&entry);
		*eb = entry;
	}
	if (endstr == strlen(path)) {
		return *eb;
	}
	else {
		/* cek apakah direktori atau bukan */
		if ((*eb).Atribut & 0x8) {
			ptr_block index;
			index = (*eb).IndexFirst;	
			entry_block next = createEntryBlock(index, 0);
			return getEntry(&next, path + endstr);
		}
		else {
			return createEntryBlockEmpty();
		}
	}
}

/** Mengembalikan entry kosong selanjutnya. Jika blok penuh, akan dibuatkan entri baru */
entry_block getNextEmptyEntry(entry_block * this) {
	entry_block entry = *this;
	while (!isEmpty(&entry)) {
		entry = nextEntry(&entry);
	}
	if (entry.Position == END_BLOCK) {
		/* berarti blok saat ini sudah penuh, buat blok baru */
		ptr_block newPosition = allocateBlock();
		ptr_block lastPos = (*this).Position;
		while (filesys.NextBlock[lastPos] != END_BLOCK) {
			lastPos = filesys.NextBlock[lastPos];
		}
		setNextBlock(lastPos, newPosition);
		entry.Position = newPosition;
		entry.Offset = 0;
	}
	return entry;
}
/** Memeriksa apakah Entry kosong atau tidak */
int isEmpty(entry_block * eb) {
	return *((*eb).Name) == 0;
}

time_t getDateTime(entry_block * eb) {
	time_t rawtime;
	time(&rawtime);
	struct tm *result = localtime(&rawtime);
	result->tm_sec = (*eb).Time[1] & 0x1F;
	result->tm_min = (((*eb).Time[1] >> 5) | ((*eb).Time[0] << 3)) & 0x3F;
	result->tm_hour = ((*eb).Time[0] >> 3u) & 0x1F;
	result->tm_mday = (*eb).Date[1] & 0x1F;
	result->tm_mon = ((*eb).Date[1] >> 5u) & 0xF;
	result->tm_year = ((*eb).Date[0] & 0x7F) + 10;
	return mktime(result);
}
void setCurrentDateTime(entry_block * eb) {
	time_t now_t;
	time(&now_t);
	struct tm *now = localtime(&now_t);
	int sec = now->tm_sec;
	int min = now->tm_min;
	int hour = now->tm_hour;
	int day = now->tm_mday;
	int mon = now->tm_mon;
	int year = now->tm_year;
	(*eb).Time[1] = ((sec >> 1) | (min << 5)) & 0xFF;
	(*eb).Time[0] = (min>>3 | (hour << 3));
	(*eb).Date[1] = ((day) | (mon << 5)) & 0xFF;
	(*eb).Date[0] = (mon >>3) | ((year - 10) << 1);
}
/** Menuliskan entry ke filesystem */
void writeEntryBlock(entry_block * eb) {
	if ((*eb).Position != END_BLOCK) {
		fseek(stream, BLOCK_SIZE * DATA_POOL_OFFSET + (*eb).Position * BLOCK_SIZE + (*eb).Offset * ENTRY_SIZE, SEEK_SET);
		fwrite(&(*eb).Name, sizeof(char), 21, stream); 		// Name
		fwrite(&(*eb).Atribut, sizeof(char), 1, stream); 		// Atribut
		fwrite((*eb).Time, sizeof(char), 2, stream);		// Time
		fwrite((*eb).Date, sizeof(char), 2, stream);		// Date
		fwrite(&(*eb).IndexFirst, sizeof(unsigned short), 1, stream); // IndexFirst
		fwrite(&(*eb).Size, sizeof(int), 1, stream);		// Size
	}
}

void makeEmpty(entry_block * eb) {
	/* menghapus byte pertama data */
	memset(eb, 0, sizeof(*eb));
}