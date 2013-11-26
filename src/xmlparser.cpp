/*
 *
 * Author Vlad Seryakov vlad@crystalballinc.com
 *   
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://mozilla.org/.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * XML parser was taken from xine project and converted into C++ and reentrant
 *
 */

#include "lmbox.h"

#define NAME_SIZE                       256
#define TOKEN_SIZE                      16 * 1024
#define MAX_RECURSION                   64

#define STATE_IDLE                      0
#define STATE_NODE                      1
#define STATE_COMMENT                   7

#define NORMAL                          0          // normal lex mode
#define DATA                            1          // data lex mode
#define CDATA                           2          // cdata lex mode

#define T_ERROR         -1      // lexer error
#define T_EOF            0      // end of file
#define T_EOL            1      // end of line
#define T_SEPAR          2      // separator ' ' '/t' '\n' '\r'
#define T_M_START_1      3      // markup start <
#define T_M_START_2      4      // markup start </
#define T_M_STOP_1       5      // markup stop >
#define T_M_STOP_2       6      // markup stop />
#define T_EQUAL          7      // =
#define T_QUOTE          8      // \" or \'
#define T_STRING         9      // "string"
#define T_IDENT         10      // identifier
#define T_DATA          11      // data
#define T_C_START       12      // <!--
#define T_C_STOP        13      // -->
#define T_TI_START      14      // <?
#define T_TI_STOP       15      // ?>
#define T_DOCTYPE_START 16      // <!DOCTYPE
#define T_DOCTYPE_STOP  17      // >
#define T_CDATA_START   18      // <![CDATA[
#define T_CDATA_STOP    19      // ]]>

static struct {
    char code;
    unsigned char namelen;
    char name[6];
} lexer_entities[] = {
    { '"',   4, "quot" },
    { '&',   3, "amp"  },
    { '\'',  4, "apos" },
    { '<',   2, "lt"   },
    {'>',    2, "gt"   },
    { 0,     0, ""     }
};

XML_Property::XML_Property(const char *name, const char *value)
{
    this->name = lmbox_strdup(name);
    this->value = lmbox_strdup(value);
    this->next = NULL;
}

XML_Property::~XML_Property(void)
{
    lmbox_free(name);
    lmbox_free(value);
}

XML_Node::XML_Node(const char *name, const char *data)
{
    this->name = lmbox_strdup(name);
    this->data = lmbox_strdup(data);
    this->properties = NULL;
    this->child = NULL;
    this->next = NULL;
}

XML_Node::~XML_Node(void)
{
    lmbox_free(name);
    lmbox_free(data);
    if (next) {
        delete next;
    }
    if (child) {
        delete child;
    }
    while (properties) {
        XML_Property *next = properties->next;
        delete properties;
        properties = next;
    }
}

int XML_Node::GetPropertyInt(const char *name)
{
    char *value = GetProperty(name);
    return value ? atoi(value) : 0;
}

char *XML_Node::GetProperty(const char *name)
{
    XML_Property *prop = properties;
    while(prop) {
        if (!strcasecmp(prop->name, name)) {
            return prop->value;
        }
        prop = prop->next;
    }
    return NULL;
}

XML_Parser::XML_Parser(const char *buf, int size)
{
    lex_bufpos = 0;
    lex_bufsize = size;
    lex_buf = (char*)buf;
    lex_mode = NORMAL;
    lex_file = 0;
    in_comment = 0;
    lex_line = 0;
    arg = 0;
    startcb = endcb = datacb = 0;
    root = new XML_Node("xmldata", 0);
    if (lex_buf) {
        ParseNode(root, "", 0);
    }
}

XML_Parser::XML_Parser(const char *file)
{
    lex_bufpos = 0;
    lex_bufsize = 0;
    lex_file = lmbox_strdup(file);
    lex_buf = strFileRead(file, &lex_bufsize);
    lex_mode = NORMAL;
    lex_line = 0;
    in_comment = 0;
    arg = 0;
    startcb = endcb = datacb = 0;
    root = new XML_Node("xmlfile", file);
    if (lex_buf) {
        ParseNode(root, "", 0);
    }
}

XML_Parser::~XML_Parser(void)
{
    if (!strcmp(root->name, "xmlfile")) {
        lmbox_free(lex_buf);
    }
    lmbox_free(lex_file);
    delete root;
}

XML_Node *XML_Parser::GetRoot(void)
{
    if (!root->next && root->child) {
        return root->child;
    }
    return root;
}

const char *XML_Parser::GetFile(void)
{
    return lex_file;
}

void XML_Parser::SetCallbacks(void *arg, xmlcallbackfn *startcb, xmlcallbackfn *endcb, xmlcallbackfn *datacb)
{
    this->arg = arg;
    this->startcb = startcb;
    this->endcb = endcb;
    this->datacb = datacb;
}

void XML_Parser::PrintNode(XML_Node * node, int indent)
{
    XML_Node *n;
    XML_Property *p;

    for (int i = 0; i < indent; i++) printf(" ");
    printf("<%s ", node->name);
    for (p = node->properties; p; p = p->next) {
        printf("%s='%s' ", p->name, p->value);
    }
    printf(">\n");
    if (node->data) {
        printf("%s\n",node->data);
    }
    if (node->child) {
        PrintNode(node->child, indent + 2);
    }
    for (int i = 0; i < indent; i++) printf(" ");
    printf("</%s>\n", node->name);
    if (node->next) {
        PrintNode(node->next, indent);
    }
}

void XML_Parser::WalkNode(XML_Node * node)
{
    startcb(arg, node);
    if (node->data) {
        datacb(arg, node);
    }
    if (node->child) {
        WalkNode(node->child);
    }
    endcb(arg, node);
    if (node->next) {
        WalkNode(node->next);
    }
}

int XML_Parser::ParseNode(XML_Node * node, char *root_name, int rec)
{
    char tok[TOKEN_SIZE];
    char property_name[NAME_SIZE];
    char node_name[NAME_SIZE];
    int state = STATE_IDLE;
    int res = 0, parse_res;
    int bypass_get_token = 0;
    XML_Node *subtree = NULL;
    XML_Node *current_subtree = NULL;
    XML_Property *property = NULL;
    XML_Property *properties = NULL;
    int debug = systemObject->configGetInt("xml_debug", 0);

    if (rec < MAX_RECURSION) {
        while(bypass_get_token || (res = ParseToken(tok, TOKEN_SIZE)) != T_ERROR) {
            bypass_get_token = 0;
            if (debug) {
                systemObject->Log(0, "xmlparser: state=%d, res=%d, pos=%d, line=%d: %s", state, res, lex_bufpos, lex_line, tok);
            }
            switch(state) {
             case STATE_IDLE:
                 switch(res) {
                  case T_EOL:
                  case T_SEPAR:
                      /* do nothing */
                      break;
                  case T_EOF:
                      return 0;   /* normal end */
                      break;
                  case T_M_START_1:
                      state = STATE_NODE;
                      break;
                  case T_M_START_2:
                      state = 3;
                      break;
                  case T_C_START:
                      state = STATE_COMMENT;
                      break;
                  case T_TI_START:
                      state = 8;
                      break;
                  case T_DOCTYPE_START:
                      state = 9;
                      break;
                  case T_CDATA_START:
                      state = 10;
                      break;
                  case T_CDATA_STOP:
                  case T_DATA:
                      /* current data */
                      if (node->data) {
                          lmbox_free(node->data);
                      }
                      node->data = ParseEntity(tok);
                      break;
                  default:
                      systemObject->Log(0, "Found unexpected token '%s'(state=%d, res=%d, pos=%d, line=%s:%d)", tok, state, res, lex_bufpos, lex_file, lex_line);
                      return -1;
                      break;
                 }
                 break;

             case STATE_NODE:
                 switch(res) {
                  case T_IDENT:
                      properties = NULL;
                      property = NULL;
                      snprintf(node_name, sizeof(node_name), tok);
                      state = 2;
                      break;
                  default:
                      systemObject->Log(0, "Found unexpected token '%s'(state=%d, res=%d, pos=%d, line=%s:%d)", tok, state, res, lex_bufpos, lex_file, lex_line);
                      return -1;
                      break;
                 }
                 break;

             case 2:
                 switch(res) {
                  case T_EOL:
                  case T_SEPAR:
                      /* nothing */
                      break;
                  case T_M_STOP_1:
                      /* new subtree */
                      subtree = new XML_Node(node_name, 0 );
                      /* set node propertys */
                      subtree->properties = properties;
                      parse_res = ParseNode(subtree, node_name, rec + 1);
                      if (parse_res != 0) {
                          return parse_res;
                      }
                      if (current_subtree == NULL) {
                          node->child = subtree;
                          current_subtree = subtree;
                      } else {
                          current_subtree->next = subtree;
                          current_subtree = subtree;
                      }
                      state = STATE_IDLE;
                      break;
                  case T_M_STOP_2:
                      /* new leaf */
                      /* new subtree */
                      subtree = new XML_Node(node_name, 0);
                      /* set node propertys */
                      subtree->properties = properties;
                      if (current_subtree == NULL) {
                          node->child = subtree;
                          current_subtree = subtree;
                      } else {
                          current_subtree->next = subtree;
                          current_subtree = subtree;
                      }
                      state = STATE_IDLE;
                      break;
                  case T_IDENT:
                      /* save property name */
                      snprintf(property_name, sizeof(property_name), "%s", tok);
                      state = 5;
                      break;
                  default:
                      systemObject->Log(0, "Found unexpected token '%s'(state=%d, res=%d, pos=%d, line=%s:%d)", tok, state, res, lex_bufpos, lex_file, lex_line);
                      return -1;
                      break;
                 }
                 break;

             case 3:
                 switch(res) {
                  case T_IDENT:
                      /* must be equal to root_name */
                      if (strcmp(tok, root_name) == 0) {
                          state = 4;
                      } else {
                          systemObject->Log(0, "Found unexpected token '%s'. Expected token '%s'(state=%d, res=%d, pos=%d, line=%s:%d)", tok, root_name, state, res, lex_bufpos, lex_file, lex_line);
                          return -1;
                      }
                      break;
                  default:
                      systemObject->Log(0, "Found unexpected token '%s'(state=%d, res=%d, pos=%d, line=%s:%d)", tok, state, res, lex_bufpos, lex_file, lex_line);
                      return -1;
                      break;
                 }
                 break;

                 /* > expected */
             case 4:
                 switch(res) {
                  case T_M_STOP_1:
                      return 0;
                      break;
                  default:
                      systemObject->Log(0, "Found unexpected token '%s'(state=%d, res=%d, pos=%d, line=%s:%d)", tok, state, res, lex_bufpos, lex_file, lex_line);
                      return -1;
                      break;
                 }
                 break;

                 /* = or > or ident or separator expected */
             case 5:
                 switch(res) {
                  case T_EOL:
                  case T_SEPAR:
                      /* do nothing */
                      break;
                  case T_EQUAL:
                      state = 6;
                      break;
                  case T_IDENT:
                      bypass_get_token = 1;       /* jump to state 2 without get a new token */
                      state = 2;
                      break;
                  case T_M_STOP_1:
                      /* add a new property without value */
                      if (property == NULL) {
                          properties = new XML_Property(property_name, 0);
                          property = properties;
                      } else {
                          property->next = new XML_Property(property_name, 0);
                          property = property->next;
                      }
                      bypass_get_token = 1;       /* jump to state 2 without get a new token */
                      state = 2;
                      break;
                  default:
                      systemObject->Log(0, "Found unexpected token '%s'(state=%d, res=%d, pos=%d, line=%s:%d)", tok, state, res, lex_bufpos, lex_file, lex_line);
                      return -1;
                 }
                 break;

                 /* string or ident or separator expected */
             case 6:
                 switch(res) {
                  case T_EOL:
                  case T_SEPAR:
                      /* do nothing */
                      break;
                  case T_STRING:
                  case T_IDENT:
                      /* add a new property */
                      if (property == NULL) {
                          properties = new XML_Property(property_name, ParseEntity(tok));
                          property = properties;
                      } else {
                          property->next = new XML_Property(property_name, ParseEntity(tok));
                          property = property->next;
                      }
                      state = 2;
                      break;
                  default:
                      systemObject->Log(0, "Found unexpected token '%s'(state=%d, res=%d, pos=%d, line=%s:%d)", tok, state, res, lex_bufpos, lex_file, lex_line);
                      return -1;
                 }
                 break;

                 /* --> expected */
             case STATE_COMMENT:
                 switch(res) {
                  case T_C_STOP:
                      state = STATE_IDLE;
                      break;
                  default:
                      state = STATE_COMMENT;
                      break;
                 }
                 break;

                 /* > expected */
             case 8:
                 switch(res) {
                  case T_TI_STOP:
                      state = 0;
                      break;
                  default:
                      state = 8;
                      break;
                 }
                 break;

                 /* ?> expected */
             case 9:
                 switch(res) {
                  case T_M_STOP_1:
                      state = 0;
                      break;
                  default:
                      state = 9;
                      break;
                 }
                 break;

                 /* ]]> expected */
             case 10:
                 switch(res) {
                  case T_CDATA_STOP:
                      bypass_get_token = 1;
                      state = 0;
                      break;
                  default:
                      state = 10;
                      break;
                 }
                 break;

             default:
                 systemObject->Log(0, "We are in an unknown parser state(state=%d, res=%d, pos=%d, line=%s:%d)", state, res, lex_bufpos, lex_file, lex_line);
                 return -1;
            }
        }
        lex_buf[lex_bufpos+80 > lex_bufsize ? lex_bufsize : lex_bufpos+80] = 0;
        systemObject->Log(0, "Lexer error state=%d at pos=%d line=%d %s", state, lex_bufpos, lex_line, &lex_buf[lex_bufpos]);
        return -1;
    } else {
        /* max recursion */
        systemObject->Log(0, "The maximum recursion depth(%d) has been reached at pos=%d line=%s;%d", MAX_RECURSION, lex_bufpos, lex_file, lex_line);
        return -1;
    }
}

int XML_Parser::ParseToken(char *tok, int tok_size)
{
    int tok_pos = 0, state = 0;

    while (tok_pos < tok_size && lex_bufpos < lex_bufsize) {
        char c = lex_buf[lex_bufpos];
        if (c == '\n') {
            lex_line++;
        }
        if (lex_mode == NORMAL) {
            /* normal mode */
            switch(state) {
             case 0:
                 switch(c) {
                  case '\n':
                  case '\r':
                      state = 1;
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;

                  case ' ':
                  case '\t':
                      state = 2;
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;

                  case '<':
                      state = 3;
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;

                  case '>':
                      state = 4;
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;

                  case '/':
                      if (!in_comment) {
                          state = 5;
                      }
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;

                  case '=':
                      state = 6;
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;

                  case '\'':
                  case '\"': /* " */
                      state = 7;
                      break;

                  case '-':
                      state = 10;
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;

                  case '?':
                      state = 9;
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;

                  default:
                      state = 100;
                      tok[tok_pos] = c;
                      tok_pos++;
                      break;
                 }
                 lex_bufpos++;
                 break;

                 /* end of line */
             case 1:
                 if (c == '\n' || c == '\r') {
                     tok[tok_pos] = c;
                     lex_bufpos++;
                     tok_pos++;
                 } else {
                     tok[tok_pos] = '\0';
                     return T_EOL;
                 }
                 break;

                 /* T_SEPAR */
             case 2:
                 if (c == ' ' || c == '\t') {
                     tok[tok_pos] = c;
                     lex_bufpos++;
                     tok_pos++;
                 } else {
                     tok[tok_pos] = '\0';
                     return T_SEPAR;
                 }
                 break;

                 /* T_M_START < or </ or <! or <? */
             case 3:
                 switch(c) {
                  case '/':
                      tok[tok_pos] = c;
                      lex_bufpos++;
                      tok_pos++;      /* FIXME */
                      tok[tok_pos] = '\0';
                      return T_M_START_2;
                      break;
                  case '!':
                      tok[tok_pos] = c;
                      lex_bufpos++;
                      tok_pos++;
                      state = 8;
                      break;
                  case '?':
                      tok[tok_pos] = c;
                      lex_bufpos++;
                      tok_pos++;      /* FIXME */
                      tok[tok_pos] = '\0';
                      return T_TI_START;
                      break;
                  default:
                      tok[tok_pos] = '\0';
                      return T_M_START_1;
                 }
                 break;

                 /* T_M_STOP_1 */
             case 4:
                 tok[tok_pos] = '\0';
                 if (!in_comment) {
                     lex_mode = DATA;
                 }
                 return T_M_STOP_1;
                 break;

                 /* T_M_STOP_2 */
             case 5:
                 if (c == '>') {
                     tok[tok_pos] = c;
                     lex_bufpos++;
                     tok_pos++;  /* FIXME */
                     tok[tok_pos] = '\0';
                     if (!in_comment) {
                         lex_mode = DATA;
                     }
                     return T_M_STOP_2;
                 } else {
                     tok[tok_pos] = '\0';
                     return T_ERROR;
                 }
                 break;

                 /* T_EQUAL */
             case 6:
                 tok[tok_pos] = '\0';
                 return T_EQUAL;
                 break;

                 /* T_STRING */
             case 7:
                 tok[tok_pos] = c;
                 lex_bufpos++;
                 if (c == '\"' || c == '\'') {   /* " */
                     tok[tok_pos] = '\0';        /* FIXME */
                     return T_STRING;
                 }
                 tok_pos++;
                 break;

                 /* T_C_START, T_DOCTYPE_START, T_CDATA_START */
             case 8:
                 switch(c) {
                  case '-':
                      lex_bufpos++;
                      if (lex_buf[lex_bufpos] == '-') {
                          lex_bufpos++;
                          tok[tok_pos++] = '-';       /* FIXME */
                          tok[tok_pos++] = '-';
                          tok[tok_pos] = '\0';
                          in_comment = 1;
                          return T_C_START;
                      }
                      break;
                  case 'D':
                      lex_bufpos++;
                      if (!strncmp(lex_buf + lex_bufpos, "OCTYPE", 6)) {
                          strncpy(tok + tok_pos, "DOCTYPE", 7);      /* FIXME */
                          lex_bufpos += 6;
                          return T_DOCTYPE_START;
                      } else {
                          return T_ERROR;
                      }
                      break;
                  case '[':
                      lex_bufpos++;
                      if (!strncmp(lex_buf + lex_bufpos, "CDATA[", 6)) {
                          lex_bufpos += 6;
                          lex_mode = CDATA;
                          return T_CDATA_START;
                      } else {
                          return T_ERROR;
                      }
                  default:
                      /* error */
                      return T_ERROR;
                 }
                 break;

                 /* T_TI_STOP */
             case 9:
                 if (c == '>') {
                     tok[tok_pos] = c;
                     lex_bufpos++;
                     tok_pos++;  /* FIXME */
                     tok[tok_pos] = '\0';
                     return T_TI_STOP;
                 } else {
                     tok[tok_pos] = '\0';
                     return T_ERROR;
                 }
                 break;

                 /* -- */
             case 10:
                 switch(c) {
                  case '-':
                      tok[tok_pos] = c;
                      tok_pos++;
                      lex_bufpos++;
                      state = 11;
                      break;
                  default:
                      tok[tok_pos] = c;
                      tok_pos++;
                      lex_bufpos++;
                      state = 100;
                 }
                 break;

                 /* --> */
             case 11:
                 switch(c) {
                  case '>':
                      tok[tok_pos] = c;
                      tok_pos++;
                      lex_bufpos++;
                      tok[tok_pos] = '\0';    /* FIX ME */
                      if (strlen(tok) != 3) {
                          tok[tok_pos - 3] = '\0';
                          lex_bufpos -= 3;
                          return T_IDENT;
                      } else {
                          in_comment = 0;
                          return T_C_STOP;
                      }
                      break;
                  default:
                      tok[tok_pos] = c;
                      tok_pos++;
                      lex_bufpos++;
                      state = 100;
                 }
                 break;

                 /* IDENT */
             case 100:
                 switch(c) {
                  case '<':
                  case '>':
                  case '\\':
                  case '\'':
                  case '\"':
                  case ' ':
                  case '\n':
                  case '\r':
                  case '\t':
                  case '=':
                  case '/':
                      tok[tok_pos] = '\0';
                      return T_IDENT;
                      break;
                  case '?':
                      tok[tok_pos] = c;
                      tok_pos++;
                      lex_bufpos++;
                      state = 9;
                      break;
                  case '-':
                      tok[tok_pos] = c;
                      tok_pos++;
                      lex_bufpos++;
                      state = 10;
                      break;
                  default:
                      tok[tok_pos] = c;
                      tok_pos++;
                      lex_bufpos++;
                 }
                 break;
             default:
                 systemObject->Log(0, "Expected char \'%c\' at pos=%d, line=%s:%d", tok[tok_pos - 1], lex_bufpos, lex_file, lex_line);
                 return T_ERROR;
            }
        } else
        if (lex_mode == DATA) {
            /* data mode, stop if char equal '<' */
            switch(c) {
             case '<':
                 tok[tok_pos] = '\0';
                 lex_mode = NORMAL;
                 return T_DATA;
             default:
                 tok[tok_pos] = c;
                 tok_pos++;
                 lex_bufpos++;
            }
        } else
        if (lex_mode == CDATA) {
            /* cdata mode, stop on ]]> */
            switch(c) {
             case '>':
                 if (tok_pos > 1 && tok[tok_pos-1] == ']' && tok[tok_pos-2] == ']') {
                     tok[tok_pos-2] = '\0';
                     lex_mode = NORMAL;
                     lex_bufpos++;
                     return T_CDATA_STOP;
                 }
             default:
                 tok[tok_pos] = c;
                 tok_pos++;
                 lex_bufpos++;
            }
        }
    }
    if (tok_pos >= tok_size) {
        systemObject->Log(0,"Token buffer is too small: pos=%d, state=%d, mode=%d: %d >= %d, line=%s:%d", lex_bufpos, state, lex_mode, tok_pos, tok_size, lex_file, lex_line);
    } else {
        if (lex_bufpos >= lex_bufsize) {
            /* Terminate the current token */
            tok[tok_pos] = '\0';
            switch(state) {
             case 0:
             case 1:
             case 2:
                 return T_EOF;
                 break;
             case 3:
                 return T_M_START_1;
                 break;
             case 4:
                 return T_M_STOP_1;
                 break;
             case 5:
                 return T_ERROR;
                 break;
             case 6:
                 return T_EQUAL;
                 break;
             case 7:
                 return T_STRING;
                 break;
             case 100:
                 return T_DATA;
                 break;
             default:
                 systemObject->Log(0,"Unknown state, state=%d pos=%d line=%s:%d", state, lex_bufpos, lex_file, lex_line);
            }
        } else {
            systemObject->Log(0,"Abnormal end of buffer, state=%d pos=%d line=%s:%d", state, lex_bufpos, lex_file, lex_line);
        }
    }
    return T_ERROR;
}

char *XML_Parser::ParseEntity(char *tok)
{
    char *buf = (char*)malloc(strlen(tok) + 1);
    char c, *bp = buf;

    while((c = *tok++)) {
        if (c != '&') {
            *bp++ = c;
        } else {
            /* parse the character entity(on failure, treat it as literal text) */
            char *tp = tok;
            long i;
            for (i = 0; lexer_entities[i].code; ++i) {
                if (!strncmp(lexer_entities[i].name, tok, lexer_entities[i].namelen) &&
                    tok[lexer_entities[i].namelen] == ';') {
                    break;
                }
            }
            if (lexer_entities[i].code) {
                tok += lexer_entities[i].namelen + 1;
                *bp++ = lexer_entities[i].code;
                continue;
            }
            if (*tp++ != '#') {
                /* not a recognised name and not numeric */
                *bp++ = '&';
                continue;
            }
            /* entity is a number
             *(note: strtol() allows "0x" prefix for hexadecimal, but we don't)
             */
            if (*tp == 'x' && tp[1] && tp[2] != 'x') {
                i = strtol(tp + 1, &tp, 16);
            } else {
                i = strtol(tp, &tp, 10);
            }
            if (i < 1 || i > 255 || *tp != ';') {
                /* out of range, or format error */
                *bp++ = '&';
                continue;
            }
            tok = tp + 1;
            *bp++ = i;
        }
    }
    *bp = 0;
    return buf;
}
