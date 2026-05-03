#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct buffer {
  size_t size;
  char *data;
};

bool buffer_is_equal(struct buffer b1, struct buffer b2) {
  if (b1.size != b2.size) {
    return false;
  }

  for (int i = 0; i < b1.size; ++i) {
    if (b1.data[i] != b2.data[i]) {
      return false;
    }
  }

  return true;
}

enum json_token_type {
  TOKEN_ERROR,

  TOKEN_OPEN_BRACE,
  TOKEN_OPEN_BRACKET,
  TOKEN_CLOSE_BRACE,
  TOKEN_CLOSE_BRACKET,
  TOKEN_COLON,
  TOKEN_COMMA,

  TOKEN_STRING_LITERAL,
  TOKEN_NUMBER,
  TOKEN_FALSE,
  TOKEN_TRUE,
  TOKEN_NULL,
};

struct json_token {
  enum json_token_type type;
  struct buffer value;
};

struct json_element {
  struct buffer label;
  struct buffer value;
  struct json_element *first_sub_element;
  struct json_element *next_element;
};

struct json_parser {
  struct buffer source;
  uint64_t at;
};

bool is_json_whitespace(char ch) {
  return ((ch == ' ') || (ch == '\n') || (ch == '\r') || (ch == '\t'));
}

bool is_json_digit(char ch) { return ch >= '0' && ch <= '9'; }

void parse_json_keyword(struct buffer source, uint64_t *at,
                        struct json_token *result, struct buffer remaining,
                        enum json_token_type type) {
  if (source.size - *at >= remaining.size) {
    struct buffer origin = source;
    origin.data += *at;
    origin.size = remaining.size;
    if (buffer_is_equal(origin, remaining)) {
      result->type = type;
      result->value.size += remaining.size;
      *at += remaining.size;
    }
  }
}

struct json_token get_json_token(struct json_parser *parser) {
  struct buffer source = parser->source;
  uint64_t at = parser->at;

  while (is_json_whitespace(source.data[at])) {
    ++at;
  }

  struct json_token result = {};

  if (at < source.size) {
    result.type = TOKEN_ERROR;
    result.value.size = 1;
    result.value.data = source.data + at;

    switch (source.data[at++]) {
    case '{':
      result.type = TOKEN_OPEN_BRACE;
      break;

    case '}':
      result.type = TOKEN_CLOSE_BRACE;
      break;

    case '[':
      result.type = TOKEN_OPEN_BRACKET;
      break;

    case ']':
      result.type = TOKEN_CLOSE_BRACKET;
      break;

    case ':':
      result.type = TOKEN_COLON;
      break;

    case ',':
      result.type = TOKEN_COMMA;
      break;

    case '"': {
      result.type = TOKEN_STRING_LITERAL;
      uint64_t str_start = at;

      while (source.data[at] != '"') {
        if (source.data[at] == '\\' && source.data[at + 1] == '"') {
          ++at;
        }
        ++at;
      }

      result.value.data = source.data + str_start;
      result.value.size = at - str_start;
      ++at;
      break;
    }

    case 'f': {
      struct buffer remaining = {.data = "alse", .size = 4};
      parse_json_keyword(source, &at, &result, remaining, TOKEN_FALSE);

      break;
    }

    case 't': {
      struct buffer remaining = {.data = "rue", .size = 3};
      parse_json_keyword(source, &at, &result, remaining, TOKEN_TRUE);
      break;
    }

    case 'n': {
      result.type = TOKEN_NULL;
      struct buffer remaining = {.data = "ull", .size = 3};
      parse_json_keyword(source, &at, &result, remaining, TOKEN_NULL);

      break;
    }

    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      uint64_t start = --at;
      result.type = TOKEN_NUMBER;

      if (source.data[at] == '-') {
        ++at;
      }

      if (source.data[at] != '0') {
        while (at < source.size && is_json_digit(source.data[at])) {
          ++at;
        }
      } else {
        ++at;
      }

      if (at < source.size && source.data[at] == '.') {
        ++at;
        while (at < source.size && is_json_digit(source.data[at])) {
          ++at;
        }
      }

      if (at < source.size &&
          (source.data[at] == 'e' || source.data[at] == 'E')) {
        ++at;

        if (at < source.size &&
            (source.data[at] == '-' || source.data[at] == '+')) {
          ++at;
        }

        while (at < source.size && is_json_digit(source.data[at])) {
          ++at;
        }
      }

      result.value.size = at - start;

      break;
    }
    }
  }
  parser->at = at;

  return result;
}

struct json_element *parse_json_list(struct json_parser *parser,
                                     enum json_token_type end_type,
                                     bool hasLabels);

struct json_element *parse_json_element(struct json_parser *parser,
                                        struct buffer label,
                                        struct json_token token) {

  struct json_element *sub_element = 0;

  if (token.type == TOKEN_ERROR) {
    return 0;
  } else if (token.type == TOKEN_OPEN_BRACE) {
    sub_element = parse_json_list(parser, TOKEN_CLOSE_BRACE, true);
  } else if (token.type == TOKEN_OPEN_BRACKET) {
    sub_element = parse_json_list(parser, TOKEN_CLOSE_BRACKET, false);
  }

  struct json_element *result = malloc(sizeof(*result));
  result->label = label;
  result->value = token.value;
  result->first_sub_element = sub_element;
  result->next_element = 0;

  return result;
}

struct json_element *parse_json_list(struct json_parser *parser,
                                     enum json_token_type end_type,
                                     bool hasLabels) {
  struct json_element *first_element = {};
  struct json_element *last_element = {};

  while (true) {
    struct json_token token = get_json_token(parser);
    if (token.type == end_type) {
      break;
    }

    struct buffer label = {};

    if (hasLabels) {
      label = token.value;

      // should be colon
      token = get_json_token(parser);

      // shoud be value
      token = get_json_token(parser);
    }

    struct json_element *element = parse_json_element(parser, label, token);
    if (element) {
      if (last_element) {
        last_element->next_element = element;
        last_element = last_element->next_element;
      } else {
        first_element = element;
        last_element = first_element;
      }
    } else if (token.type == end_type) {
      break;
    } else {
      printf("unexpected token, at: %ld", parser->at);
      break;
    }

    token = get_json_token(parser);
    if (token.type == end_type) {
      break;
    } else if (token.type != TOKEN_COMMA) {
      printf("unexpected character after value, at: %ld, ch: %c, tv: %.*s, td: "
             "%d\n",
             parser->at, parser->source.data[parser->at], (int)token.value.size,
             token.value.data, token.type);
      exit(1);
    }
  }

  return first_element;
}

struct json_element *parse_json(struct buffer input) {
  struct json_parser parser = {};
  parser.source = input;

  struct json_element *element =
      parse_json_element(&parser, (struct buffer){}, get_json_token(&parser));
  return element;
}
