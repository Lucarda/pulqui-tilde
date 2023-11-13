#include "m_pd.h"

static t_class *pulqui_tilde_class;

typedef struct _pulqui_tilde {
  t_object x_obj;
  t_float x_val;
  t_float f;

  t_inlet *x_in2;
  t_inlet *x_in3;
  t_outlet*x_out;
} t_pulqui_tilde;  


static t_int *pulqui_tilde_perform(t_int *w)
{
  t_pulqui_tilde *x = (t_pulqui_tilde *)(w[1]);
  t_sample    *in1 =      (t_sample *)(w[2]);
  t_sample    *in2 =      (t_sample *)(w[3]);
  t_sample    *out =      (t_sample *)(w[4]);
  int            n =             (int)(w[5]);
  t_sample val = x->x_val;

  while (n--) *out++ = (*in1++) * (*in2++);

  return (w+6);
}

static void pulqui_tilde_dsp(t_pulqui_tilde *x, t_signal **sp)
{
  dsp_add(pulqui_tilde_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void pulqui_tilde_free(t_pulqui_tilde *x)
{
  inlet_free(x->x_in2);
  //inlet_free(x->x_in3);
  outlet_free(x->x_out);
}

static void *pulqui_tilde_new(t_floatarg f)
{
  t_pulqui_tilde *x = (t_pulqui_tilde *)pd_new(pulqui_tilde_class);

  x->x_val = f;

  x->x_in2=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  //x->x_in3=floatinlet_new (&x->x_obj, &x->x_val);
  x->x_out=outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void pulqui_tilde_setup(void) 
{
  pulqui_tilde_class = class_new(gensym("testobject~"),
        (t_newmethod)pulqui_tilde_new,
        (t_method)pulqui_tilde_free, sizeof(t_pulqui_tilde),
        CLASS_DEFAULT,
        A_DEFFLOAT, 0);

  class_addmethod(pulqui_tilde_class,
        (t_method)pulqui_tilde_dsp, gensym("dsp"), A_CANT, 0);
  CLASS_MAINSIGNALIN(pulqui_tilde_class, t_pulqui_tilde, f);
}