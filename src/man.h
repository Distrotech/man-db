/* man.c */
static void man_getopt (int argc, char *argv[]);
static __inline__ const char *is_section (const char *name);
static __inline__ char **get_section_list (void);
static int man (const char *name);
static int display (const char *dir, const char *man_file,
		    const char *cat_file, const char *title,
		    const char *dbfilters);
static __inline__ char *make_roff_command (const char *dir, const char *file,
					   const char *dbfilters);
static __inline__ int do_prompt (const char *name);

/* convert_name.c */
extern char *convert_name (const char *name, const char *alternate);
