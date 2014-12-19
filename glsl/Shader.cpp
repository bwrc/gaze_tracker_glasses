#include "Shader.h"
#include <stdio.h>
#include <fstream>
#include <string.h>


static char *readSrc(const std::string &file);


Shader::Shader() {

    idProg     = 0;
    idVertex   = 0;
    idFragment = 0;

}


Shader::~Shader() {

    if(idProg != 0) {

        glDetachShader(idProg, idVertex);
        glDetachShader(idProg, idFragment);
 
        glDeleteShader(idVertex);
        glDeleteShader(idFragment);
        glDeleteProgram(idProg);

    }

}


bool Shader::create(const std::string &vertFile,
                    const std::string &fragFile) {

    GLenum err;

    idVertex = glCreateShader(GL_VERTEX_SHADER);
    if(idVertex == 0) {

        err = glGetError();

        printf("\n\n**************************************************\n");
        printf("idVertex = glCreateShader(GL_VERTEX_SHADER);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

    }

    idFragment = glCreateShader(GL_FRAGMENT_SHADER);	

    if(idFragment == 0) {

        err = glGetError();

        printf("\n\n**************************************************\n");
        printf("idFragment = glCreateShader(GL_FRAGMENT_SHADER);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

    }

    char *vertexSrc = readSrc(vertFile);

    if(vertexSrc == NULL) {

        delete[] vertexSrc;

        printf("Could not read vertex source %s\n", vertFile.c_str());

        return false;

    }


    char *fragSrc = readSrc(fragFile);

    if(fragSrc == NULL) {

        delete[] fragSrc;

        printf("Could not read fragment source %s\n", fragFile.c_str());
        return false;

    }


    glShaderSource(idVertex, 1, (const char **)&vertexSrc, NULL);
    err = glGetError();
    if(err != GL_NO_ERROR) {

        printf("\n\n**************************************************\n");
        printf("glShaderSource(idVertex, 1, (const char **)&vertexSrc, NULL);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

        delete[] vertexSrc;
        delete[] fragSrc;

        return false;

    }


    glShaderSource(idFragment, 1, (const char **)&fragSrc, NULL);
    err = glGetError();
    if(err != GL_NO_ERROR) {

        printf("\n\n**************************************************\n");
        printf("glShaderSource(idFragment, 1, (const char **)&fragmentSrc, NULL);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

        delete[] vertexSrc;
        delete[] fragSrc;

        return false;

    }

    delete[] vertexSrc;
    delete[] fragSrc;


    glCompileShader(idVertex);
    int bVertexCompiled;
    glGetShaderiv(idVertex, GL_COMPILE_STATUS, &bVertexCompiled);
    if(bVertexCompiled == GL_FALSE) {

        err = glGetError();
        printf("\n\n**************************************************\n");
        printf("glCompileShader(idVertex);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

        return false;

    }

    glCompileShader(idFragment);
    int bFragmentCompiled;
    glGetShaderiv(idFragment, GL_COMPILE_STATUS, &bFragmentCompiled);
    if(bFragmentCompiled == GL_FALSE) {

        err = glGetError();

        printf("\n\n**************************************************\n");
        printf("glCompileShader(idFragment);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

        return false;

    }


    idProg = glCreateProgram();
    if(idProg == 0) {

        err = glGetError();

        printf("\n\n**************************************************\n");
        printf("idProg = glCreateProgram();  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

        return false;

    }


    glAttachShader(idProg, idVertex);
    err = glGetError();
    if(err != GL_NO_ERROR) {

        printf("\n\n**************************************************\n");
        printf("glAttachShader(idProg, idVertex);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

        return false;

    }

    glAttachShader(idProg, idFragment);
    err = glGetError();
    if(err != GL_NO_ERROR) {

        printf("\n\n**************************************************\n");
        printf("glAttachShader(idProg, idFragment);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

        return false;

    }

    glLinkProgram(idProg);
    int bLinked;
    glGetProgramiv(idProg, GL_LINK_STATUS, &bLinked);
    if(bLinked == GL_FALSE) {

        err = glGetError();

        printf("\n\n**************************************************\n");
        printf(" glLinkProgram(idProg);  %s\n", gluErrorString(err));
        printf("**************************************************\n\n");

        return false;

    }

    return true;

}


bool Shader::use() {

    glUseProgram(idProg);

    return true;

}


char *readSrc(const std::string &file) {

    std::ifstream in;
    in.open(file.c_str(), std::ifstream::in);
    if(!in.is_open()) {
        return NULL;
    }

    in.seekg(0, std::ios::end);
    int n = in.tellg();
    in.seekg(0, std::ios::beg);

    char *src = new char[n + 1];

	memset(src, '\0', n+1);

    in.read(src, n);


    return src;

}

