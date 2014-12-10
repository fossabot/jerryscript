/* Copyright 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "literal.h"
#include "ecma-helpers.h"
#include "jerry-libc.h"

literal
create_empty_literal (void)
{
  return (literal)
  {
    .type = LIT_UNKNOWN,
    .data.none = NULL
  };
}

literal
create_literal_from_num (ecma_number_t num)
{
  return (literal)
  {
    .type = LIT_NUMBER,
    .data.num = num
  };
}

literal
create_literal_from_str (const char *s, ecma_length_t len)
{
  return create_literal_from_zt ((const ecma_char_t *) s, len);
}

literal
create_literal_from_str_compute_len (const char *s)
{
  return create_literal_from_zt ((const ecma_char_t *) s, (ecma_length_t) __strlen (s));
}

literal
create_literal_from_zt (const ecma_char_t *s, ecma_length_t len)
{
  for (ecma_magic_string_id_t msi = 0; msi < ECMA_MAGIC_STRING__COUNT; msi++)
  {
    if (ecma_zt_string_length (ecma_get_magic_string_zt (msi)) != len)
    {
      continue;
    }
    if (!__strncmp ((const char *) s, (const char *) ecma_get_magic_string_zt (msi), len))
    {
      return (literal)
      {
        .type = LIT_MAGIC_STR,
        .data.magic_str_id = msi
      };
    }
  }
  return (literal)
  {
    .type = LIT_STR,
    .data.lp = (lp_string)
    {
      .length = len,
      .str = s,
      .hash = ecma_chars_buffer_calc_hash_last_chars (s, len)
    }
  };
}

bool
literal_equal_type (literal lit1, literal lit2)
{
  if (lit1.type != lit2.type)
  {
    return false;
  }
  return literal_equal (lit1, lit2);
}

bool
literal_equal_type_s (literal lit, const char *s)
{
  return literal_equal_type_zt (lit, (const ecma_char_t *) s);
}

bool
literal_equal_type_zt (literal lit, const ecma_char_t *s)
{
  if (lit.type != LIT_STR && lit.type != LIT_MAGIC_STR)
  {
    return false;
  }
  return literal_equal_zt (lit, s);
}

bool
literal_equal_type_num (literal lit, ecma_number_t num)
{
  if (lit.type != LIT_NUMBER)
  {
    return false;
  }
  return literal_equal_num (lit, num);
}

static bool
literal_equal_lp (literal lit, lp_string lp)
{
  switch (lit.type)
  {
    case LIT_UNKNOWN:
    {
      return false;
    }
    case LIT_STR:
    {
      return lp_string_equal (lit.data.lp, lp);
    }
    case LIT_MAGIC_STR:
    {
      return lp_string_equal_zt (lp, ecma_get_magic_string_zt (lit.data.magic_str_id));
    }
    case LIT_NUMBER:
    {
      ecma_char_t buff[ECMA_MAX_CHARS_IN_STRINGIFIED_NUMBER];
      ecma_number_to_zt_string (lit.data.num, buff, ECMA_MAX_CHARS_IN_STRINGIFIED_NUMBER);
      return lp_string_equal_zt (lp, buff);
    }
    default:
    {
      JERRY_UNREACHABLE ();
    }
  }
}

bool
literal_equal (literal lit1, literal lit2)
{
  switch (lit2.type)
  {
    case LIT_UNKNOWN:
    {
      return lit2.type == LIT_UNKNOWN;
    }
    case LIT_STR:
    {
      return literal_equal_lp (lit1, lit2.data.lp);
    }
    case LIT_MAGIC_STR:
    {
      return literal_equal_zt (lit1, ecma_get_magic_string_zt (lit2.data.magic_str_id));
    }
    case LIT_NUMBER:
    {
      return literal_equal_num (lit1, lit2.data.num);
    }
    default:
    {
      JERRY_UNREACHABLE ();
    }
  }
}

bool
literal_equal_s (literal lit, const char *s)
{
  return literal_equal_zt (lit, (const ecma_char_t *) s);
}

bool
literal_equal_zt (literal lit, const ecma_char_t *s)
{
  switch (lit.type)
  {
    case LIT_UNKNOWN:
    {
      return false;
    }
    case LIT_STR:
    {
      return lp_string_equal_zt (lit.data.lp, s);
    }
    case LIT_MAGIC_STR:
    {
      return ecma_compare_zt_strings (s, ecma_get_magic_string_zt (lit.data.magic_str_id));
    }
    case LIT_NUMBER:
    {
      ecma_char_t buff[ECMA_MAX_CHARS_IN_STRINGIFIED_NUMBER];
      ecma_number_to_zt_string (lit.data.num, buff, ECMA_MAX_CHARS_IN_STRINGIFIED_NUMBER);
      return ecma_compare_zt_strings (s, buff);
    }
    default:
    {
      JERRY_UNREACHABLE ();
    }
  }
}

bool
literal_equal_num (literal lit, ecma_number_t num)
{
  ecma_char_t buff[ECMA_MAX_CHARS_IN_STRINGIFIED_NUMBER];
  ecma_number_to_zt_string (num, buff, ECMA_MAX_CHARS_IN_STRINGIFIED_NUMBER);
  return literal_equal_zt (lit, buff);
}

const ecma_char_t *
literal_to_zt (literal lit)
{
  JERRY_ASSERT (lit.type == LIT_STR || lit.type == LIT_MAGIC_STR);

  switch (lit.type)
  {
    case LIT_STR: return lit.data.lp.str;
    case LIT_MAGIC_STR: return ecma_get_magic_string_zt (lit.data.magic_str_id);
    default: JERRY_UNREACHABLE ();
  }
}
