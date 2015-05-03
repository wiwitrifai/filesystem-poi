/* FIle : rp_fuse.c */

#include "rp_fuse.h"

extern FILE * stream;
extern poi_file filesys;
extern time_t mount_time;

/** Get file attributes.*/
int rp_poi_getattr(const char* path, struct stat* stbuf) {
	/* jika root path */
	if (strcmp(path, "/") == 0) {
		stbuf->st_nlink = 1;
		stbuf->st_mode = S_IFDIR | 0777; // file dengan permission rwxrwxrwx
		stbuf->st_mtime = mount_time;
		return 0;
	}
	else {
		entry_block empty = createEntryBlockEmpty();
		entry_block entry = getEntry(&empty,path);
		//Kalau path tidak ditemukan
		if (isEmpty(&entry)) {
			return -ENOENT;
		}
		// tulis stbuf, tempat memasukkan atribut file
		stbuf->st_nlink = 1;
		// cek direktori atau bukan
		if (entry.Atribut & 0x8) {
			stbuf->st_mode = S_IFDIR | (0770 + (entry.Atribut & 0x7));
		}
		else {
			stbuf->st_mode = S_IFREG | (0660 + (entry.Atribut & 0x7));
		}
		// ukuran file
		stbuf->st_size = entry.Size;
		// waktu pembuatan file
		stbuf->st_mtime = getDateTime(&entry);
		return 0;
	}
}
/** Read directory */
int rp_poi_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	// current & parent directory
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	entry_block empty = createEntryBlockEmpty();
	entry_block entry = getEntry(&empty, path);
	ptr_block index = entry.IndexFirst;
	entry = createEntryBlock(index, 0);
	// fungsi filler digunakan untuk setiap entry pada direktori tsb
	// ditulis ke buffer "buf"
	while (entry.Position != END_BLOCK) {
		if(!isEmpty(&entry)){
			filler(buf, entry.Name, NULL, 0);
		}
		entry = nextEntry(&entry);
	}
	return 0;
}
/** Create a directory */
int rp_poi_mkdir(const char *path, mode_t mode) {

	/* mencari parent directory */
	int i;
	for(i = strlen(path)-1; path[i] != '/'; i--);
	char parentPath[21];
	strncpy(parentPath, path, i);
	entry_block entry;
	//bagi kasus kalau dia root
	if (strcmp(parentPath, "") == 0) {
		entry = createEntryBlockEmpty();
	}
	else {
		entry = getEntry(&entry, parentPath);
		ptr_block index = entry.IndexFirst;
		entry = createEntryBlock(index, 0);
	}
	/* mencari entry kosong di parent */
	entry = getNextEmptyEntry(&entry);
	/* menuliskan data di entry tersebut */
	strcpy(entry.Name, path + i + 1);
	entry.Atribut = 0x0F;
	setCurrentDateTime(&entry);
	entry.IndexFirst = allocateBlock();
	entry.Size = 0x00;
	writeEntryBlock(&entry);
	return 0;
	//jangan lupa urusin mode

}

/** File open operation */
int rp_poi_open(const char* path, struct fuse_file_info* fi){
	/* hanya mengecek apakah file ada atau tidak */
	entry_block entry;
	entry = getEntry(&entry, path);
	if(isEmpty(&entry)) {
		return -ENOENT;
	}
	return 0;

}

/** Remove a directory */
int rp_poi_rmdir(const char *path){
	/* mencari entry dengan nama path */
	entry_block entry;
	entry = getEntry(&entry, path);
	if(isEmpty(&entry)){
		return -ENOENT;
	}
	/* masuk ke direktori dari indeks */
	/* menghapus dari tiap allocation table */
	freeBlock(entry.IndexFirst);
	//removeDir(entry.getIndex());
	makeEmpty(&entry);
	return 0;

}

/** Rename a file */
int rp_poi_rename(const char* path, const char* newpath) {
	entry_block empty = createEntryBlockEmpty();
	entry_block entryAsal = getEntry(&empty, path);
	entry_block entryLast = getNewEntry(&empty, newpath);
	if(!isEmpty(&entryAsal)){
		//entryLast.setName(entryAsal.getName().c_str());
		entryLast.Atribut = entryAsal.Atribut;
		entryLast.IndexFirst = entryAsal.IndexFirst;
		entryLast.Size = entryAsal.Size;
		entryLast.Time[0] = entryAsal.Time[0];
		entryLast.Time[1] = entryAsal.Time[1];
		entryLast.Date[0] = entryAsal.Date[0];
		entryLast.Date[1] = entryAsal.Date[1];
		writeEntryBlock(&entryLast);
		/* set entry asal jadi kosong */
		makeEmpty(&entryLast);
	}
	else
		return -ENOENT;
	
	return 0;

}

/** Remove a file */
int rp_poi_unlink(const char *path){
	entry_block empty = createEntryBlockEmpty();
	entry_block entry = getEntry(&empty, path);
	if(entry.Atribut & 0x8){
		return -ENOENT;
	}
	else{
		freeBlock(entry.IndexFirst);
		makeEmpty(&entry);
	}
	return 0;

}

int rp_poi_truncate(const char *path, off_t newsize) {
	entry_block empty = createEntryBlockEmpty();
	entry_block entry = getEntry(&empty,path);
	
	/* set sizenya */
	entry.Size = newsize;
	writeEntryBlock(&entry);
	
	/* urusin allocation table */
	ptr_block position = entry.IndexFirst;
	while (newsize > 0) {
		newsize -= BLOCK_SIZE;
		if (newsize > 0) {
			/* kalau gak cukup, alokasiin baru */
			if (filesys.NextBlock[position] == END_BLOCK) {
				setNextBlock(position, allocateBlock());
			}
			position = filesys.NextBlock[position];
		}
	}
	freeBlock(filesys.NextBlock[position]);
	setNextBlock(position, END_BLOCK);
	
	return 0;
}

int rp_poi_read (const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	//menuju ke entry
	entry_block empty = createEntryBlockEmpty();
	entry_block entry = getEntry(&empty,path);
	ptr_block index = entry.IndexFirst;
	
	//kalo namanya kosong
	if(isEmpty(&entry)){
		return -ENOENT;
	}
	
	//read
	return readBlock(index,buf,size,offset);
	
}

int rp_poi_write(const char *path,const char *buf,size_t size,off_t offset,struct fuse_file_info *fi){
	entry_block empty = createEntryBlockEmpty();
	entry_block entry = getEntry(&empty,path);
	ptr_block index = entry.IndexFirst;
	
	//kalo namanya kosong
	if(isEmpty(&entry)){
		return -ENOENT;
	}
	
	entry.Size = offset + size;
	writeEntryBlock(&entry);
	
	int result = writeBlock(index, buf, size, offset);
	
	return result;
}

int rp_poi_link(const char *path, const char *newpath) {
	entry_block empty = createEntryBlockEmpty();
	entry_block oldentry = getEntry(&empty, path);
	
	/* kalo nama kosong */
	if(isEmpty(&oldentry)){
		return -ENOENT;
	}
	/* buat entry baru dengan nama newpath */
	entry_block newentry = getNewEntry(&empty, newpath);
	/* set atribut untuk newpath */
	newentry.Atribut = oldentry.Atribut;
	setCurrentDateTime(&newentry);
	newentry.Size = oldentry.Size;
	writeEntryBlock(&newentry);
	
	/* copy isi file */
	char buffer[4096];
	/* lakukan per 4096 byte */
	int totalsize = oldentry.Size;
	int offset = 0;
	while (totalsize > 0) {
		int sizenow = totalsize;
		if (sizenow > 4096) {
			sizenow = 4096;
		}
		readBlock(oldentry.IndexFirst, buffer, oldentry.Size, offset);
		writeBlock(newentry.IndexFirst, buffer, newentry.Size, offset);
		totalsize -= sizenow;
		offset += 4096;
	}
	
	return 0;
}

/** Create a node file */
int rp_poi_mknod(const char *path, mode_t mode, dev_t dev);
