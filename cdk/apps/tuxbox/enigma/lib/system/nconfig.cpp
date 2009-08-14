// this is nconfig 0.92, a bit modified
#define NO_MAP_SHARED
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <lib/system/nconfig.h>

#include <fcntl.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include <sys/mman.h>

#include <limits.h>

#ifndef PAGESIZE
#ifdef PAGE_SIZE
#define PAGESIZE PAGE_SIZE
#else
#define PAGESIZE 4096
#endif
#endif

#define SB				((struct nc_sb_s *) data)
#define CM(x)			((struct nc_chunk_s *) (data+(x)))
#define DE(x)			((struct nc_de_s *) (data+(x)))
#define IDE(x, y)		(DE(((unsigned *) (data+(x)->offset))[(y)]))
#define CS(x)			(((unsigned *) (data+(x)))[-1])


NConfig::NConfig(int protect)
{
	fd = -1;
	cname = fname = data = NULL;
	sb = NULL;
	cdir = NULL;
	chunks = NULL;
	revision = update = lsize = omode = 0;
	olck = 0;
	lock = NC_L_NONE;
	careful = protect;
}

NConfig::~NConfig()
{
	close();
	free(fname);
}

void NConfig::close()
{
	free(cname);
	cname = NULL;
	if (fd > -1) {
#ifdef NO_MAP_SHARED
		if (data) {
			int size=sb->size;
			char *buffer=new char[size];
			memcpy(buffer, data, size);
			munmap(data, size);
			data = NULL;
			::lseek(fd, 0, SEEK_SET);
			::write(fd, buffer, size);
			delete[] buffer;
		}
#endif
		::close(fd);
		fd = -1;
	}
	if (data) {
		munmap(data, sb->size);
		data = NULL;
	}
}

void NConfig::flush()
{
	close();
	open(omode);
}

int NConfig::setName(const char *name)
{
	if (!name)
		return NC_ERR_NVAL;
	if (fd > -1)
		return NC_ERR_PERM;
	free(fname);
	fname = strdup(name);
	return NC_ERR_OK;
}

int NConfig::open(int how)
{
	if (!fname)
		return NC_ERR_NFILE;
	if (how != NC_O_RO && how != NC_O_RW)
		return NC_ERR_TYPE;
	if (fd > -1)
		close();

	int ff;
	if ((ff = ::open(fname, how)) == -1)
		return NC_ERR_PERM;

	struct stat sbuf;
	fstat(ff, &sbuf);

	if (!sbuf.st_size)
		return NC_ERR_CORRUPT;

#ifdef NO_MAP_SHARED
	if ((data = (char *) mmap(NULL, sbuf.st_size, how == NC_O_RO ? PROT_READ : (PROT_READ|PROT_WRITE), MAP_PRIVATE, ff, 0)) == MAP_FAILED) {
#else
	if ((data = (char *) mmap(NULL, sbuf.st_size, how == NC_O_RO ? PROT_READ : (PROT_READ|PROT_WRITE), MAP_SHARED, ff, 0)) == MAP_FAILED) {
#endif
		::close(ff);
		return NC_ERR_NMEM;
	}
	if (memcmp(((struct nc_sb_s *) data)->magic, NC_SB_MAGIC, 4)) {
		munmap(data, sbuf.st_size);
		::close(ff);
		return NC_ERR_CORRUPT;
	}
	fd = ff;
	omode = how;
	sb = SB;
	lsize = 0;
	cname = strdup("/");

	lockFile(NC_L_RO, TRUE);
	rdir = DE(sb->root);
	unLockFile();
	return NC_ERR_OK;
}

struct nc_de_s *NConfig::getDirEnt(const char *name, struct nc_de_s *cc)
{
	struct nc_de_s *ret = cc ? cc : ((*name == '/') ? rdir : cdir);
	char *c = canonize(name), *can;

	if (!(can = c))
		return ret;
	while (*c) {
		if (!strcmp(c, ".."))
			ret = DE(ret->parent);
		else
			if (strcmp(c, ".")) {
				struct nc_de_s *re = ret;
				int left = 0, right = ret->pages-1, p, r;

				ret = NULL;
				while (left <= right) {
					p = (left + right) / 2;
					r = strcmp(c, data+IDE(re, p)->name);
					if (r < 0) {
						left = p + 1;
						continue;
					}
					if (!r) {
						ret = IDE(re, p);
						break;
					}
					right = p - 1;
				}
			}
		c += strlen(c)+1;
		if (!ret || (*c && ret->type != NC_DIR)) {
			ret = NULL;
			break;
		}
	}
	free(can);
	return ret;
}

char *NConfig::canonize(const char *name)
{
	if (*name == '/')
		name++;
	size_t i = strlen(name);
	char *ret = (char *)calloc(1, i+3);
	memcpy(ret, name, i);
	for (size_t j=0; j<i; j++)
		if (ret[j] == '/')
			ret[j] = 0;
	return ret;
}

void NConfig::lockFile(int type, int force)
{
#ifdef NC_DEBUG_LOCK
	fprintf(stderr, "Lock called type=%d force=%d lock=%d olck=%u\n", type, force, lock, olck);
#endif
	if (lock == NC_L_RO && type == NC_L_RW) {
		fprintf(stderr, "Lock promotion is not possible.\n");
		abort();
	}
	if (lock != NC_L_NONE) {
		olck++;
		return;
	}
	
	struct flock flc = { type == NC_L_RW ? F_WRLCK : F_RDLCK, SEEK_SET, 0, 0, 0 };
	while (fcntl(fd, F_SETLKW, &flc)) {
		sched_yield();
		flc.l_type = type == NC_L_RW ? F_WRLCK : F_RDLCK;
		flc.l_whence = SEEK_SET;
		flc.l_len = flc.l_start = 0;
	}

#ifdef NC_DEBUG_LOCK
	fprintf(stderr, "Locked %u %u %s\n", sb->modtime, update, force ? "forced." : "");
#endif
	if (careful && type == NC_L_RW)
		mprotect(data, sb->size, PROT_READ | PROT_WRITE);
	lock = type;
	olck = 0;
	if (sb->modtime != update || force) {
		// refresh memory mapping
		if (lsize != sb->size) {
			_remap(lsize, sb->size);
			lsize = sb->size;
			chunks = CM(sb->chunk);
		}
		cdir = getDirEnt(cname);
		update = sb->modtime;
	}
}

void NConfig::unLockFile()
{
#ifdef NC_DEBUG_LOCK
	fprintf(stderr, "UnLock called lock=%u olck=%u\n", lock, olck);
#endif
	if (olck) {
		olck--;
		return;
	}
	if (lock == NC_L_NONE)
		return;
	struct flock flc = {F_UNLCK, SEEK_SET, 0, 0, 0 };
	update = sb->modtime;
#ifdef NC_DEBUG_LOCK
	fprintf(stderr, "Unlock %u\n", update);
#endif
	if (careful)
		mprotect(data, sb->size, PROT_READ);
	fcntl(fd, F_SETLK, &flc);
	lock = NC_L_NONE;
	olck = 0;
}

void NConfig::_remap(const size_t osize, const size_t nsize)
{
	data = (char *) mremap(data, osize, nsize, 1);
	if (data == MAP_FAILED) {
		perror("mremap");
		abort();
	}
	sb = SB;
	rdir = DE(sb->root);
}

void NConfig::store(nc_de_s *de, FILE *f, eString path)
{
	struct nc_de_s *cc;
	for (unsigned i=0; i<de->pages; i++)
		if ((cc = IDE(de, i))->type)
		{
			switch (cc->type)
			{
			case NC_DIR:
			{
				eString tmp=path;
				tmp+='/';
				tmp+=((char*)(data+cc->name));
				store(cc, f, tmp);
				break;
			}
			case NC_STRING:
			{
				eString tmp=path;
				tmp+='/';
				tmp+=((char*)(data+cc->name));
				fprintf(f, "s:%s=%s\n", tmp.c_str(), (char*)(data+cc->offset) );
				break;
			}
			case NC_INT:
			{
				eString tmp=path;
				tmp+='/';
				tmp+=((char*)(data+cc->name));
				int bla = *((signed long long *) (data+cc->offset));
				fprintf(f, "i:%s=%08x\n", tmp.c_str(), bla );
				break;
			}
			case NC_UINT:
			{
				eString tmp=path;
				tmp+='/';
				tmp+=((char*)(data+cc->name));
				unsigned int bla = *((unsigned long long *) (data+cc->offset));
				fprintf(f, "u:%s=%08x\n", tmp.c_str(), bla );
				break;
			}
			case NC_DOUBLE:
			{
				eString tmp=path;
				tmp+='/';
				tmp+=((char*)(data+cc->name));
				double bla = *((long double *) (data+cc->offset));
				fprintf(f, "d:%s=%lf\n", tmp.c_str(), bla );
				break;
			}
			case NC_RAW:
				break;
			}
		}
}

int NConfig::convert(const char *filename)
{
	if (fd < 0)
		return NC_ERR_NFILE;

	FILE *f = fopen(filename, "w");
	if (!f)
		return NC_ERR_PERM;

	lockFile(NC_L_RO);
	store(rdir, f, "");
	unLockFile();

	fclose(f);
	return NC_ERR_OK;
}

eConfigOld::eConfigOld()
{
	setName(CONFIGDIR "/enigma/registry");
	open();
}

eConfigOld::~eConfigOld()
{
	struct stat s;
	if (!::stat("/var/tuxbox/config/enigma/registry", &s))
		system("mv /var/tuxbox/config/enigma/registry /var/tuxbox/config/enigma/registry_unneeded");
}
