const char* statbuf_get_perms(struct stat *sbuf)
{
	static char perms[] = "----------";
	perms[0] = '?';

	mode_t mode = sbuf->st_mode;
	switch (mode & S_IFMT)
	{
	case S_IFREG:
		perms[0] = '-';
		break;
	case S_IFDIR:
		perms[0] = 'd';
		break;
	case S_IFLNK:
		perms[0] = 'l';
		break;
	case S_IFIFO:
		perms[0] = 'p';
		break;
	case S_IFSOCK:
		perms[0] = 's';
		break;
	case S_IFCHR:
		perms[0] = 'c';
		break;
	case S_IFBLK:
		perms[0] = 'b';
		break;
	}

	if (mode & S_IRUSR)
	{
		perms[1] = 'r';
	}
	if (mode & S_IWUSR)
	{
		perms[2] = 'w';
	}
	if (mode & S_IXUSR)
	{
		perms[3] = 'x';
	}
	if (mode & S_IRGRP)
	{
		perms[4] = 'r';
	}
	if (mode & S_IWGRP)
	{
		perms[5] = 'w';
	}
	if (mode & S_IXGRP)
	{
		perms[6] = 'x';
	}
	if (mode & S_IROTH)
	{
		perms[7] = 'r';
	}
	if (mode & S_IWOTH)
	{
		perms[8] = 'w';
	}
	if (mode & S_IXOTH)
	{
		perms[9] = 'x';
	}
	if (mode & S_ISUID)
	{
		perms[3] = (perms[3] == 'x') ? 's' : 'S';
	}
	if (mode & S_ISGID)
	{
		perms[6] = (perms[6] == 'x') ? 's' : 'S';
	}
	if (mode & S_ISVTX)
	{
		perms[9] = (perms[9] == 'x') ? 't' : 'T';
	}

	return perms;
}

const char* statbuf_get_date(struct stat *sbuf)
{static char datebuf[64] = {0};
	const char *p_date_format = "%b %e %H:%M";
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t local_time = tv.tv_sec;
	if (sbuf->st_mtime > local_time || (local_time - sbuf->st_mtime) > 60*60*24*182)
	{
		p_date_format = "%b %e  %Y";
	}

	struct tm* p_tm = localtime(&local_time);
	strftime(datebuf, sizeof(datebuf), p_date_format, p_tm);

	return datebuf;
}