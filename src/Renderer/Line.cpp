#include <Renderer/Line.hpp>
#include <Renderer.hpp>


static GLuint VBO;
static GLuint VAO;

void Line::init() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);


  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(0 * sizeof(float)));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


Line::Line() {}

void Line::update() {}
void Line::draw() const {
  const unsigned int indeciesC = indecies.size();

  std::vector<float> data((3+4) * indeciesC);

  for(int i = 0; i < indeciesC; i++) {
    const unsigned int tri = indecies[i];
    const unsigned int start = (3+4) * i;

    const Vec3& curVert = vertices[tri];
    data[start+0] = curVert.x;
    data[start+1] = curVert.y;
    data[start+2] = curVert.z;


    static constexpr float BYTE2FLOAT = 1/255.0f;

    const Color& curColor = colors[tri];
    data[start+3] = curColor.r * BYTE2FLOAT;
    data[start+4] = curColor.g * BYTE2FLOAT;
    data[start+5] = curColor.b * BYTE2FLOAT;
    data[start+6] = curColor.a * BYTE2FLOAT;
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(VAO);

  glDrawArrays(GL_LINES, 0, indeciesC);
}