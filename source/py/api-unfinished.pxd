## DOES NOT IMPORT!!!


cdef extern from "ext_boxstyle.h":

    cdef void class_attr_setstyle(t_class *c, const char *s)
    cdef void class_attr_style_alias(t_class *c, const char *name, const char *aliasname, long legacy)
    cdef int FILL_ATTR_SAVE
    cdef void class_attr_setfill(t_class *c, const char *name, long flags)
    cdef void jgraphics_attr_fillrect(t_object *b, t_jgraphics *g, t_symbol *attrname, t_rect *area)
    cdef t_jpattern *jgraphics_attr_setfill(t_object *b, t_jgraphics *g, t_symbol *attrname, t_rect *area)
    cdef void object_attr_getfillcolor_atposition(t_object *b, const char *attrname, double pos, t_jrgba *c)
    cdef long object_attr_getfill(t_object *obj, t_symbol *attrname)
    cdef void object_style_setfillattribute(t_object *x, t_symbol *fillattr, t_symbol *entry, long argc, t_atom *argv)
    cdef void class_attr_stylemap(t_class *c, char *attrname, char *mapname)
    cdef t_symbol *object_attr_attrname_forstylemap(t_object *x, t_symbol *mapname)     
    cdef t_symbol *object_attr_stylemapname(t_object *x, t_symbol *attrname)            
    cdef t_jpopupmenu *style_getmenu(t_object *context, t_symbol *current, long mask, long *selecteditem, long *headercount)        
    cdef void style_handlemenu(t_object *context, long itemindex, t_symbol **current)

    cdef void CLASS_ATTR_STYLE_RGBA_NOSAVE(c,attrname,flags,structname,structmember,label)
    cdef void CLASS_ATTR_STYLE_RGBA(c,attrname,flags,structname,structmember,label)
    cdef void CLASS_ATTR_STYLE_RGBA_PREVIEW(c,attrname,flags,structname,structmember,label,previewtype)
    cdef void CLASS_ATTR_STYLE_ALIAS_NOSAVE(c,attrname,aliasname)
    cdef void CLASS_ATTR_STYLE_ALIAS_COMPATIBILITY(c,attrname,aliasname)
    cdef void CLASS_ATTR_STYLE_ALIAS_RGBA_LEGACY(c,attrname,aliasname)




cdef extern from "jgraphics.h":
    ctypedef struct t_jgraphics 
    ctypedef struct t_jpath     
    ctypedef struct t_jpattern      
    ctypedef struct t_jfont     
    ctypedef struct t_jtextlayout   
    ctypedef struct t_jtransform    
    ctypedef struct t_jsurface  
    ctypedef struct t_jdesktopui    
    ctypedef struct t_jpopupmenu    
    ctypedef struct t_jsvg      
    ctypedef struct t_jsvg_remap    

    cdef int JGRAPHICS_RECT_BOTTOM(rect)
    cdef int JGRAPHICS_RECT_RIGHT(rect)
    cdef int JGRAPHICS_PI
    cdef int JGRAPHICS_2PI
    cdef int JGRAPHICS_PIOVER2
    cdef int JGRAPHICS_3PIOVER2

    ctypedef enum t_jgraphics_line_join:
        JGRAPHICS_LINE_JOIN_MITER
        JGRAPHICS_LINE_JOIN_ROUND
        JGRAPHICS_LINE_JOIN_BEVEL


    ctypedef enum t_jgraphics_line_cap:
        JGRAPHICS_LINE_CAP_BUTT
        JGRAPHICS_LINE_CAP_ROUND
        JGRAPHICS_LINE_CAP_SQUARE


    ctypedef enum t_jgraphics_bubble_side:
        JGRAPHICS_BUBBLE_SIDE_TOP
        JGRAPHICS_BUBBLE_SIDE_LEFT
        JGRAPHICS_BUBBLE_SIDE_BOTTOM
        JGRAPHICS_BUBBLE_SIDE_RIGHT


    ctypedef enum t_jgraphics_path_type:
        JGRAPHICS_PATH_STARTNEWSUBPATH
        JGRAPHICS_PATH_LINETO
        JGRAPHICS_PATH_QUADRATICTO
        JGRAPHICS_PATH_CUBICTO
        JGRAPHICS_PATH_CLOSEPATH


    cdef int jgraphics_round(double d)

    ctypedef enum t_jgraphics_format:
        JGRAPHICS_FORMAT_ARGB32   
        JGRAPHICS_FORMAT_RGB24         
        JGRAPHICS_FORMAT_A8             

    ctypedef enum t_jgraphics_fileformat:
        JGRAPHICS_FILEFORMAT_PNG
        JGRAPHICS_FILEFORMAT_JPEG

    cdef t_jsurface* jgraphics_image_surface_create(t_jgraphics_format format, int width, int height)
    cdef t_jsurface *jgraphics_image_surface_create_referenced(const char *filename, short path)
    cdef t_jsurface* jgraphics_image_surface_create_from_file(const char *filename, short path)
    cdef t_jsurface* jgraphics_image_surface_create_for_data(unsigned char *data, t_jgraphics_format format,  int width, int height, int stride, method freefun, void *freearg)
    cdef t_jsurface* jgraphics_image_surface_create_for_data_premult(unsigned char *data, t_jgraphics_format format, int width, int height, int stride, method freefun, void *freearg)
    cdef t_jsurface* jgraphics_image_surface_create_from_filedata(const void *data, unsigned long datalen)
    cdef t_jsurface* jgraphics_image_surface_create_from_resource(const void* moduleRef, const char *resname)
    cdef t_max_err jgraphics_get_resource_data(const void *moduleRef, const char *resname, long extcount, t_atom *exts, void **data, unsigned long *datasize)
    cdef t_jsurface* jgraphics_surface_reference(t_jsurface *s)      
    cdef void        jgraphics_surface_destroy(t_jsurface *s)        
    cdef t_max_err   jgraphics_image_surface_writepng(t_jsurface *surface, const char *filename, short path, long dpi) 
    cdef t_max_err   jgraphics_image_surface_writejpeg(t_jsurface *surface, const char *filename, short path)
    cdef void        jgraphics_surface_set_device_offset(t_jsurface *s, double x_offset, double y_offset) 
    cdef void        jgraphics_surface_get_device_offset(t_jsurface *s, double *x_offset, double *y_offset) 
    cdef int         jgraphics_image_surface_get_width(t_jsurface *s)
    cdef int         jgraphics_image_surface_get_height(t_jsurface *s) 
    cdef void        jgraphics_image_surface_set_pixel(t_jsurface *s, int x, int y, t_jrgba color) 
    cdef void        jgraphics_image_surface_get_pixel(t_jsurface *s, int x, int y, t_jrgba *color)
    cdef void        jgraphics_image_surface_scroll(t_jsurface *s, int x, int y, int width, int height, int dx, int dy, t_jpath **path)      
    cdef const unsigned char* jgraphics_image_surface_lockpixels_readonly(t_jsurface *s, int x, int y, int width, int height, int *linestride, int *pixelstride)
    cdef void                 jgraphics_image_surface_unlockpixels_readonly(t_jsurface *s, const unsigned char *data) 
    cdef unsigned char*       jgraphics_image_surface_lockpixels(t_jsurface *s, int x, int y, int width, int height, int *linestride, int *pixelstride)
    cdef void                 jgraphics_image_surface_unlockpixels(t_jsurface *s, unsigned char *data) 
    cdef void        jgraphics_image_surface_draw(t_jgraphics *g, t_jsurface *s, t_rect srcRect, t_rect destRect)
    cdef void        jgraphics_image_surface_draw_fast(t_jgraphics *g, t_jsurface *s)        
    cdef void jgraphics_write_image_surface_to_filedata(t_jsurface *surf, long fmt, void **data, long *size)
    cdef t_jsurface* jgraphics_image_surface_create_from_base64(const char *base64, unsigned long datalen)
    cdef void jgraphics_write_image_surface_to_base64(t_jsurface *surf, long fmt, char **base64, long *size)
    cdef void jgraphics_image_surface_clear(t_jsurface *s, int x, int y, int width, int height)
    cdef t_jsvg*     jsvg_create_from_file(const char *filename, short path) 
    cdef t_jsvg*     jsvg_create_from_resource(const void *moduleRef, const char *resname)
    cdef t_jsvg*     jsvg_create_from_xmlstring(const char *svgXML) 
    cdef void        jsvg_get_size(t_jsvg *svg, double *width, double *height) 
    cdef void        jsvg_destroy(t_jsvg *svg) 
    cdef void        jsvg_render(t_jsvg *svg, t_jgraphics *g)
    cdef void jsvg_load_cached(t_symbol *name, t_jsvg **psvg)
    cdef t_jsvg_remap *jsvg_remap_create(t_jsvg *svg)
    cdef void jsvg_remap_addcolor(t_jsvg_remap *r, t_jrgba *src, t_jrgba *dst)
    cdef void jsvg_remap_perform(t_jsvg_remap *r, t_jsvg **remapped)
    cdef void jsvg_remap_destroy(t_jsvg_remap *r)
    cdef void jgraphics_draw_jsvg(t_jgraphics *g, t_jsvg *svg, t_rect *r, int flags, double opacity)
    cdef t_jgraphics*    jgraphics_create(t_jsurface *target)
    cdef t_jgraphics*    jgraphics_reference(t_jgraphics *g)
    cdef void            jgraphics_destroy(t_jgraphics *g)

    ctypedef struct t_jgraphics_path_elem

    cdef void        jgraphics_new_path(t_jgraphics *g)
    cdef t_jpath*    jgraphics_copy_path(t_jgraphics *g)
    cdef t_jpath*    jgraphics_path_createstroked(t_jpath *p, double thickness, t_jgraphics_line_join join, t_jgraphics_line_cap cap) 
    cdef void        jgraphics_path_destroy(t_jpath *path) 
    cdef void        jgraphics_append_path(t_jgraphics *g, t_jpath *path)
    cdef void        jgraphics_close_path(t_jgraphics *g)     
    cdef void        jgraphics_path_roundcorners(t_jgraphics *g, double cornerRadius)
    cdef long        jgraphics_path_contains(t_jpath *path, double x, double y) 
    cdef long        jgraphics_path_intersectsline(t_jpath *path, double x1, double y1, double x2, double y2) 
    cdef double      jgraphics_path_getlength(t_jpath *path) 
    cdef void        jgraphics_path_getpointalongpath(t_jpath *path, double distancefromstart, double *x, double *y) 
    cdef double      jgraphics_path_getnearestpoint(t_jpath *path, double x, double y, double *path_x, double *path_y)
    cdef long        jgraphics_path_getpathelems(t_jpath *path, t_jgraphics_path_elem **elems)
    cdef void        jgraphics_get_current_point(t_jgraphics *g, double *x, double *y) 
    cdef void        jgraphics_arc(t_jgraphics *g, double xc, double yc, double radius, double angle1, double angle2)
