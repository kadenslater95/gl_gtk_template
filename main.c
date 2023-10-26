
#include <GL/gl.h>
#include <GL/glu.h>

#include <gtk/gtk.h>

#include "file_utils.h"


int load_shader_from_file(unsigned int, char*);

unsigned int vertexShader;
unsigned int fragmentShader;

unsigned int shaderProgram;

unsigned int VAO, VBO, EBO;

float vertices[] = {
  -0.5f, -0.5f, 0.0f,
  0.5f, -0.5f, 0.0f,
  0.0f, 0.5f, 0.0f 
};

unsigned int indices[] = {  
    0, 1, 3,
    1, 2, 3  
};


static void
on_realize (GtkGLArea *area)
{
  // We need to make the context current if we want to
  // call GL API
  gtk_gl_area_make_current (area);

  GError *gl_area_error = gtk_gl_area_get_error(area);
  if(gl_area_error != NULL)
  {
    printf("Failed to create gtk_gl_area: %s\n", gl_area_error->message);
    g_error_free(gl_area_error);
    return;
  }

  GdkGLContext *context = gtk_gl_area_get_context(area);
  int major, minor;
  gdk_gl_context_get_version(context, &major, &minor);
  printf("Gtk GL Context Version: %d.%d\n", major, minor);

  printf("Using OpenGL Version: %s\n", glGetString(GL_VERSION));
  
  int  success;
  char infoLog[512];

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  load_shader_from_file(vertexShader, "./shader.vert");
  glCompileShader(vertexShader);
  
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    return;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  load_shader_from_file(fragmentShader, "./shader.frag");
  glCompileShader(fragmentShader);
  
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    return;
  }

  shaderProgram = glCreateProgram();

  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if(!success) {
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      printf("ERROR::SHADER::PROGAM::LINKAGE_FAILED\n%s\n", infoLog);
      return;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader); 


  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);


  GLenum glError = glGetError();
  if(glError != GL_NO_ERROR) {
    printf("GL ERROR: %s\n", gluErrorString(glError));
    return;
  }
}



static gboolean
render (GtkGLArea *area, GdkGLContext *context)
{
  // inside this function it's safe to use GL; the given
  // GdkGLContext has been made current to the drawable
  // surface used by the `GtkGLArea` and the viewport has
  // already been set to be the size of the allocation
  
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // we can start by clearing the buffer
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(shaderProgram);

  glBindVertexArray(VAO);

  // glDrawArrays(GL_TRIANGLES, 0, 6);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glFlush();

  GLenum glError = glGetError();
  if(glError != GL_NO_ERROR) {
    printf("GL ERROR: %s\n", gluErrorString(glError));
    return FALSE;
  }
  
  // we completed our drawing; the draw commands will be
  // flushed at the end of the signal emission chain, and
  // the buffers will be drawn on the window
  return TRUE;
}


static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *gl_area;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "GTK GL Template");
  gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

  gl_area = gtk_gl_area_new();
  
  g_signal_connect (gl_area, "realize", G_CALLBACK (on_realize), NULL);
  g_signal_connect (gl_area, "render", G_CALLBACK (render), NULL);

  gtk_window_set_child (GTK_WINDOW (window), gl_area);

  gtk_window_present (GTK_WINDOW (window));
}


int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("template.opengl.gtk.application", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}


/**
 * Read the Shader Source Code from the given file path and add it to the given shader
 * 
 * @param shader unsigned int, the uint given by glCreateShader
 * @param filePath char*, the shader source file path
 * 
 * @return int, return 1 if error is caught and 0 otherwise
*/
int load_shader_from_file(unsigned int shader, char *filePath) {
  char* fileContent;
  unsigned int fileLength;

  read_file(filePath, &fileContent, &fileLength);

  if(!fileContent) {
    printf("Failed to read file!\n");
    return 1;
  }

  glShaderSource(shader, 1, &fileContent, NULL);

  free(fileContent);

  GLenum glError = glGetError();
  if(glError != GL_NO_ERROR) {
    printf("Failed to compile shader! GL ERROR: %s\n", gluErrorString(glError));
    return FALSE;
  }

  return 0;
}

