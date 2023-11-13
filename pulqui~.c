#include "m_pd.h"

static t_class *pulqui_tilde_class;

typedef struct _pulqui_tilde {
    t_object x_obj;
    t_word *x_vec;
    int x_graphperiod;
    int x_graphcount;
    t_symbol *x_arrayname;
    t_float x_f;
    int x_npoints;
    t_sample *x_ramch
} t_pulqui_tilde;  


static void *pulqui_tilde_new(t_symbol *s)
{
    t_pulqui_tilde *x = (t_pulqui_tilde *)pd_new(pulqui_tilde_class);
    x->x_graphcount = 0;
    x->x_arrayname = s;
    x->x_f = 0;
    x->x_ramch = (t_sample *)getbytes(4096*sizeof(t_sample));
    return (x);
}


static void pulqui_tilde_set(t_pulqui_tilde *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            pd_error(x, "pulqui~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatwords(a, &x->x_npoints, &x->x_vec))
    {
        pd_error(x, "%s: bad template for pulqui~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void pq_bee32(t_pulqui_tilde *x)
{
    int pos;
    int startpos;
    int endpos;
    t_sample peakIEEE;
    startpos = 0;
    endpos = 0;
    pos = 0;
    
    LOOP:while (pos < 4096)
    {
        if ( x->x_ramch[pos] > 0) break;
        pos++;
    }
    startpos = pos;
    peakIEEE = 0;
    while (pos < 4096)
    {
        if (x->x_ramch[pos] > peakIEEE) peakIEEE = x->x_ramch[pos];
        if( x->x_ramch[pos] < 0) break;
        pos++;
    }
    endpos = pos;
    for (pos = startpos; pos < endpos ; pos++)
    {
        x->x_ramch[pos] = peakIEEE;
    }
    //endpos = pos;
    if (pos < 4096) goto LOOP;
}

static void pq_bee32_negative(t_pulqui_tilde *x)
{
    int pos;
    int startpos;
    int endpos;
    t_sample peakIEEE;
    startpos = 0;
    endpos = 0;
    pos = 0;
    
    LOOP:while (pos < 4096)
    {
        if ( x->x_ramch[pos] < 0) break;
        pos++;
    }
    startpos = pos;
    peakIEEE = 0;
    while (pos < 4096)
    {
        if (x->x_ramch[pos] < peakIEEE) peakIEEE = x->x_ramch[pos];
        if( x->x_ramch[pos] > 0) break;
        pos++;
    }
    endpos = pos;
    for (pos = startpos; pos < endpos ; pos++)
    {
        x->x_ramch[pos] = peakIEEE * -1;
    }
    //endpos = pos;
    if (pos < 4096) goto LOOP;
}


static t_int *pulqui_tilde_perform(t_int *w)
{
    t_pulqui_tilde *x = (t_pulqui_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)w[3];
    int nn = n;
    t_word *dest = x->x_vec;
    int i = x->x_graphcount;
    if (!x->x_vec) goto bad;
    if (n > x->x_npoints)
        n = x->x_npoints;
    
    int j = 0;
    while (n--)
    {
        t_sample f = *in++;
        if (PD_BIGORSMALL(f))
            f = 0;
         x->x_ramch[j] = f;
         j++;
    }
    pq_bee32(x);
    pq_bee32_negative(x);
    j = 0;
    while (nn--)
    {
        (dest++)->w_float = x->x_ramch[j];
        j++;
    }
    if (!i--)
    {
        t_garray *a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class);
        if (!a)
            bug("tabsend_dsp");
        else garray_redraw(a);
        i = x->x_graphperiod;
    }
    x->x_graphcount = i;
    
bad:
    return (w+4);
}


static void pulqui_tilde_dsp(t_pulqui_tilde *x, t_signal **sp)
{
    int n = sp[0]->s_n;
    int ticksper = sp[0]->s_sr/n;
    pulqui_tilde_set(x, x->x_arrayname);
    if (ticksper < 1) ticksper = 1;
    x->x_graphperiod = ticksper;
    if (x->x_graphcount > ticksper) x->x_graphcount = ticksper;
    dsp_add(pulqui_tilde_perform, 3, x, sp[0]->s_vec, (t_int)n);
}

static void pulqui_tilde_free(t_pulqui_tilde *x)
{
    freebytes(x->x_ramch,sizeof(4096*sizeof(t_sample)));
}


void pulqui_tilde_setup(void) 
{
    pulqui_tilde_class = class_new(gensym("pulqui~"), (t_newmethod)pulqui_tilde_new,
        0, sizeof(t_pulqui_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(pulqui_tilde_class, t_pulqui_tilde, x_f);
    class_addmethod(pulqui_tilde_class, (t_method)pulqui_tilde_dsp,
        gensym("dsp"), A_CANT, 0);
    class_addmethod(pulqui_tilde_class, (t_method)pulqui_tilde_set,
        gensym("set"), A_SYMBOL, 0);
}