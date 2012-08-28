#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

struct xml_parser{
    xmlDocPtr doc;  // pointer to parse xml Document
    xmlNodePtr cur; // node pointer. It interacts with individual node
};


int open_xmlfile(struct xml_parser *xml, char *filename, char *check) 
{
    xmlDocPtr doc;  // pointer to parse xml Document
    xmlNodePtr cur; // node pointer. It interacts with individual node

    // Parse XML file 
    doc = xmlParseFile(filename);

    // Check to see that the document was successfully parsed.
    if (doc == NULL ) {
	fprintf(stderr,"Error!. Document is not parsed successfully. \n");
	return 0;
    }

    // Retrieve the document's root element.
    cur = xmlDocGetRootElement(doc);

    // Check to make sure the document actually contains something
    if (cur == NULL) {
	fprintf(stderr,"Document is Empty\n");
	xmlFreeDoc(doc);
	return 0;
    }

    /* We need to make sure the document is the right type. 
     * "root" is the root type of the documents used in user Config XML file 
     */
    if (xmlStrcmp(cur->name, (const xmlChar *) check)) {
	fprintf(stderr,"Document is of the wrong type, root node != root");
	xmlFreeDoc(doc);
	return 0;
    }
    //cur = cur->xmlChildrenNode;

    xml->doc = doc;
    xml->cur = cur;
    return 1;
}

xmlNodePtr find_subnode(xmlNodePtr cur, char *pattern)
{
    cur = cur->xmlChildrenNode;
    while(cur != NULL){
	if ((!xmlStrcmp(cur->name, (const xmlChar *)pattern))){
	    return cur;
	}
	cur = cur->next;
    }
    return NULL;
}

#define DELIM "@"
int get_xml_node(struct xml_parser xml, char *pattern, char *prop, int index)
{
    char *ptr;
    char *token;
    xmlNodePtr cur;
    cur = xml.cur;

    char buf[100];
    strncpy(buf,pattern,strlen(pattern)+1);

    token = strtok_r(buf, DELIM, &ptr);
    int i=0;
    cur = cur->xmlChildrenNode;

    if(token && cur != NULL){
	while(cur != NULL){
	    if ((!xmlStrcmp(cur->name, (const xmlChar *)token))){
		i++;
		if(i == index)
		    break;
	    }
	    cur = cur->next;
	}
    }

    if(i != index)
	return 0;
 
    token = strtok_r(NULL, DELIM, &ptr);
    while(token){
	cur = find_subnode(cur, token);
	if(cur == NULL)
	    return 0;
	else
	    token = strtok_r(NULL, DELIM, &ptr);
	
    }

    if(cur != NULL){
	xmlChar *key;

	if(prop == NULL){
	    key = xmlNodeListGetString(xml.doc, cur->xmlChildrenNode, 1); // (3)
	    printf("result: %s\n", key);
	    xmlFree(key);
	    return 1;
	}else{
	    key = xmlGetProp(cur, prop);
	    printf("result: %s\n", key);
	    xmlFree(key);
	    return 1;
	}
    }
}

void xml_free(struct xml_parser xml){
    xmlFreeDoc(xml.doc);
    xmlCleanupParser();    //node : check plz
}

int main(int argc, char **argv) {
    struct xml_parser xml;

    if(!open_xmlfile(&xml, "ddos_object.xml","ddos")){
	printf("parsing falut\n");
	return 0;
    }

    if(!get_xml_node(xml, "object@user_score@clt_syn", "type", 1))
	printf("asdfasf\n");
    xml_free(xml);
}
