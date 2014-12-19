#ifndef SHADER_H
#define SHADER_H


#include <string>
#include <GL/glew.h>
#include <GL/gl.h>


class Shader {

public:

    Shader();
    ~Shader();

    bool create(const std::string &vertFile,
                const std::string &fragFile);

    bool use();

    GLuint getProgId() const {return idProg;}

private:

    GLuint idVertex, idFragment, idProg;

};


#endif
