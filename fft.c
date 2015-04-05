#include "fft.h"
#include "fft_internal.h"
#include <stdlib.h>
#include <stdbool.h>

struct mufft_step_1d
{
   mufft_1d_func func;
   unsigned p;
   unsigned twiddle_step;
};

struct mufft_plan_1d
{
   struct mufft_step_1d *steps;
   unsigned num_steps;

   cfloat *tmp_buffer;
   cfloat *twiddles;
};

static cfloat twiddle(int direction, int k, int p)
{
   double phase = (M_PI * direction * k) / p;
   return cos(phase) + I * sin(phase);
}

static cfloat *build_twiddles(unsigned N, int direction)
{
   cfloat *twiddles = mufft_alloc(N * sizeof(cfloat));
   if (twiddles == NULL)
   {
      return NULL;
   }

   cfloat *pt = twiddles;

   for (unsigned p = 1; p < N; p <<= 1)
   {
      for (unsigned k = 0; k < p; k++)
      {
         pt[k] = twiddle(direction, k, p);
      }
      pt += p == 2 ? 3 : p; // Make sure that twiddles for p == 4 and up are aligned properly for AVX.
   }

   return twiddles;
}

static bool add_step_1d(mufft_plan_1d *plan, const fft_step *step, unsigned p)
{
   struct mufft_step_1d *new_steps = realloc(plan->steps, (plan->num_steps + 1) * sizeof(*new_steps));
   if (new_steps != NULL)
   {
      plan->steps = new_steps;
      plan->num_steps++;
      return true;
   }
   else
   {
      return false;
   }
}

static bool build_plan_1d(mufft_plan_1d *plan, unsigned N, int direction)
{
   unsigned radix = N;
   unsigned p = 1;

   while (radix > 1)
   {
      bool found = false;

      // Find first (optimal?) routine which can do work.
      for (unsigned i = 0; i < ARRAY_SIZE(fft_table); i++)
      {
         const fft_step *step = &fft_table[i];

         bool need_specialized = p < 4;

         if (radix % step->radix == 0 &&
               N >= step->minimum_elements &&
               (!need_specialized || step->fixed_p == p))
         {
            if (add_step_1d(plan, step, p))
            {
               found = true;
               radix /= step->radix;
               p *= step->radix;
               break;
            }
         }
      }

      if (!found)
      {
         return false;
      }
   }

   return true;
}

mufft_plan_1d *mufft_create_plan_1d_c2c(unsigned N, int direction)
{
   if ((N & (N - 1)) != 0 || N == 1)
   {
      return NULL;
   }
   
   mufft_plan_1d *plan = mufft_calloc(sizeof(*plan));
   if (plan == NULL)
   {
      goto error;
   }

   plan->twiddles = build_twiddles(N, direction);
   if (plan->twiddles == NULL)
   {
      goto error;
   }

   plan->tmp_buffer = mufft_alloc(N * sizeof(cfloat));
   if (plan->tmp_buffer == NULL)
   {
      goto error;
   }

   if (!build_plan_1d(plan, N, direction))
   {
      goto error;
   }

   return plan;

error:
   mufft_free_plan_1d(plan);
   return NULL;
}

void mufft_execute_plan_1d(mufft_plan_1d *plan, void *output, const void *input)
{
}

void mufft_free_plan_1d(mufft_plan_1d *plan)
{
   if (plan == NULL)
   {
      return;
   }
   free(plan->steps);
   mufft_free(plan->tmp_buffer);
   mufft_free(plan->twiddles);
   mufft_free(plan);
}

void *mufft_alloc(size_t size)
{
   void *ptr;
   if (posix_memalign(&ptr, 64, size) < 0)
   {
      return NULL;
   }
   return ptr;
}

void *mufft_calloc(size_t size)
{
   void *ptr = mufft_alloc(size);
   if (ptr != NULL)
   {
      memset(ptr, 0, size);
   }
   return ptr;
}

void mufft_free(void *ptr)
{
   free(ptr);
}
