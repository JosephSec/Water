#include <Renderer/Mesh.hpp>
#include <Renderer.hpp>


static GLuint VBO;
static GLuint VAO;

void Mesh::init() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);


  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


Mesh::Mesh() {}

void Mesh::update() {}
void Mesh::draw() const {
  const unsigned int indeciesC = indecies.size();
  const unsigned int triC = indeciesC/3;

  std::vector<float> data((3+3+2) * indeciesC);

  for(int i = 0; i < triC; i++) {
    const unsigned int* faceStart = indecies.data() + i*3;
    const Vec3& a = vertices[*(faceStart)];
    const Vec3& b = vertices[*(faceStart+1)];
    const Vec3& c = vertices[*(faceStart+2)];

    const Vec3 normal = Vec3::cross(b-a, c-a).normalize(); //Generate Face Normal

    for(int j = 0; j < 3; j++) {
      const unsigned int start = (3+3+2) * (i*3 + j) + 3;// * i + 3(vert pos offset)
      data[start+0] = normal.x;
      data[start+1] = normal.y;
      data[start+2] = normal.z;
    }
  }

  for(int i = 0; i < indeciesC; i++) {
    const unsigned int tri = indecies[i];
    const unsigned int start = (3+3+2) * i;

    const Vec3& curVert = vertices[tri];
    data[start+0] = curVert.x;
    data[start+1] = curVert.y;
    data[start+2] = curVert.z;


    const Vec2& curUV = uvs[i];
    data[start+6] = curUV.x;
    data[start+7] = curUV.y;
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(VAO);


  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureHandle);
  Renderer::TriangleShader.SetUniform("tex", 0);

  glDrawArrays(GL_TRIANGLES, 0, indeciesC);
}