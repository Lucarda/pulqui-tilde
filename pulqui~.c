#include "m_pd.h"

static t_class *pulqui_tilde_class;

typedef struct _pulqui_tilde 
{
    t_object x_obj;
    t_float x_f; //
    t_sample *x_ramchpositive;
    t_sample *x_ramchnegative;
    t_sample *x_ramch;
    t_sample *x_bufsignal;
    t_outlet *x_out1;
    t_outlet *x_out2;
    int x_scanlen, x_len;
} t_pulqui_tilde;  


static void *pulqui_tilde_new(void)
{
    t_pulqui_tilde *x = (t_pulqui_tilde *)pd_new(pulqui_tilde_class);
    x->x_f = 0;
    x->x_len = 4096;
    x->x_scanlen = x->x_len * 2;
    x->x_out1=outlet_new(&x->x_obj, &s_signal);
    x->x_out2=outlet_new(&x->x_obj, &s_signal);
    x->x_ramchpositive = (t_sample *)getbytes(x->x_scanlen * sizeof(t_sample));
    x->x_ramchnegative = (t_sample *)getbytes(x->x_scanlen * sizeof(t_sample));
    x->x_ramch = (t_sample *)getbytes(x->x_len * sizeof(t_sample));
    x->x_bufsignal = (t_sample *)getbytes(x->x_len * sizeof(t_sample));
    return (x);
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
    
    LOOP:while (pos < x->x_scanlen)
    {
        if ( x->x_ramchpositive[pos] > 0.0001) break;
        pos++;
    }
    startpos = pos;
    peakIEEE = 0;
    while (pos < x->x_scanlen)
    {
        if (x->x_ramchpositive[pos] > peakIEEE) peakIEEE = x->x_ramchpositive[pos];
        if ( x->x_ramchpositive[pos] < 0.0001) break;
        pos++;
    }
    endpos = pos;
    for (pos = startpos; pos < endpos ; pos++)
    {
        x->x_ramchpositive[pos] = peakIEEE;
    }
    //endpos = pos;
    if (pos < x->x_scanlen) goto LOOP;
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
    
    LOOP:while (pos < x->x_scanlen)
    {
        if ( x->x_ramchnegative[pos] < -0.0001) break;
        pos++;
    }
    startpos = pos;
    peakIEEE = 0;
    while (pos < x->x_scanlen)
    {
        if (x->x_ramchnegative[pos] < peakIEEE) peakIEEE = x->x_ramchnegative[pos];
        if ( x->x_ramchnegative[pos] > -0.0001) break;
        pos++;
    }
    endpos = pos;
    for (pos = startpos; pos < endpos ; pos++)
    {
        x->x_ramchnegative[pos] = peakIEEE;
    }
    //endpos = pos;
    if (pos < x->x_scanlen) goto LOOP;
}

static t_int *pulqui_tilde_perform(t_int *w)
{
    t_pulqui_tilde *x = (t_pulqui_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out1 = (t_sample *)(w[3]);
    t_sample *out2 = (t_sample *)(w[4]);
    int n = (int)w[5];
    
    for (int i = 0; i < n; i++)
    {
        t_sample f = in[i];
        if (PD_BIGORSMALL(f))
            f = 0;
         x->x_ramch[i] = f;
    }
    
    for (int i = 0; i < n; i++)
    {
         x->x_ramchpositive[x->x_len + i] = x->x_ramch[i];
         x->x_ramchnegative[x->x_len + i] = x->x_ramch[i];
    } 
    
    pq_bee32(x);
    pq_bee32_negative(x);
    
    for (int i = 0; i < n; i++)
    {
        out1[i] = x->x_bufsignal[i];
        if (x->x_ramchpositive[i] >  0.0001)
        {
            out2[i] = x->x_ramchpositive[i];            
        }
        else if (x->x_ramchnegative[i] <  -0.0001)
        {
            out2[i] = x->x_ramchnegative[i] * -1;            
        }
        else
        {
            out2[i] = 1;
        }
    }
    
    for (int i = 0; i < n; i++)
    {
         x->x_ramchpositive[i] = x->x_ramchpositive[x->x_len + i];
         x->x_ramchnegative[i] = x->x_ramchnegative[x->x_len + i];
    }
    
    for (int i = 0; i < n; i++)
    {
        x->x_bufsignal[i] = x->x_ramch[i];
    }
    
    return (w+6);
}


static void pulqui_tilde_dsp(t_pulqui_tilde *x, t_signal **sp)
{
    dsp_add(pulqui_tilde_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void pulqui_tilde_free(t_pulqui_tilde *x)
{
    freebytes(x->x_ramch,sizeof(x->x_len*sizeof(t_sample)));
    freebytes(x->x_ramchpositive,sizeof(x->x_scanlen * sizeof(t_sample)));
    freebytes(x->x_ramchnegative,sizeof(x->x_scanlen * sizeof(t_sample)));
    freebytes(x->x_bufsignal,sizeof(x->x_len * sizeof(t_sample)));
}


void pulqui_tilde_setup(void) 
{
    pulqui_tilde_class = class_new(gensym("pulqui~"), (t_newmethod)pulqui_tilde_new,
        (t_method)pulqui_tilde_free, sizeof(t_pulqui_tilde), 0, 0);
    CLASS_MAINSIGNALIN(pulqui_tilde_class, t_pulqui_tilde, x_f);
    class_addmethod(pulqui_tilde_class, (t_method)pulqui_tilde_dsp, gensym("dsp"), A_CANT, 0);
}