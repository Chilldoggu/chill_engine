#pragma once

#include "figures.hpp"

namespace ColorsRGB {
    const std::vector<float> RED{ 1.0f, 0.0f, 0.0f };
    const std::vector<float> BLUE{ 0.0f, 0.0f, 1.0f };
    const std::vector<float> GREEN{ 0.0f, 1.0f, 0.0f };
    const std::vector<float> WHITE{ 1.0f, 1.0f, 1.0f };
    const std::vector<float> INDIGO{ 0.3f, 0.0f, 0.5f };
    const std::vector<float> HOT_PINK{ 1.0f, 0.4f, 0.7f };
    const std::vector<float> MOCCASIN{ 1.0f, 0.9f, 0.7f };
    const std::vector<float> SKY_BLUE{ 0.5f, 0.8f, 0.9f };
    const std::vector<float> LIGHT_SALMON{ 1.0f, 0.6f, 0.5f };
};

const std::vector<float> CUBE_VERT_CORDS = {
   -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f, // BACK
    0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, // FACE

   -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, // FRONT
    0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, // FACE

   -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, // LEFT
   -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, // FACE

    0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, // RIGHT
    0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, // FACE

   -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, // DOWN
    0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, // FACE

   -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f, // UP
    0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, // FACE
};

const std::vector<float> CUBE_TEXTURE_CORDS = {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};

const std::vector<float> CUBE_NORMAL_CORDS = {
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f, 
     0.0f,  0.0f, -1.0f, 
     0.0f,  0.0f, -1.0f, 
     0.0f,  0.0f, -1.0f, 
     0.0f,  0.0f, -1.0f, 

     0.0f,  0.0f, 1.0f,
     0.0f,  0.0f, 1.0f,
     0.0f,  0.0f, 1.0f,
     0.0f,  0.0f, 1.0f,
     0.0f,  0.0f, 1.0f,
     0.0f,  0.0f, 1.0f,

    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,

     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,

     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,

     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f
};

const std::vector<std::vector<int>> CUBE_COLOR_ORDER = {
    { 6, 11, 16, 28 },	      // front bottom left  vertex
    { 7, 22, 26, 27 },	      // front bottom right vertex
    { 8, 9, 18, 23, 32, 33 }, // front top    right vertex
    { 10, 12, 17, 34 },	      // front top    left  vertex
    { 0, 5, 14, 15, 24, 29 }, // back  bottom left  vertex
    { 1, 20, 21, 25 },	      // back  bottom right vertex
    { 2, 3, 19, 31 },	      // back  top    right vertex
    { 4, 13, 30, 35 }	      // back  top    left  vertex
};

namespace Materials {
    const Material GOLD      { glm::vec3(0.247250, 0.199500, 0.074500), glm::vec3(0.751640, 0.606480, 0.226480), glm::vec3(0.628281, 0.555802, 0.366065), 0.40000000 * 128 };
    const Material JADE      { glm::vec3(0.135000, 0.222500, 0.157500), glm::vec3(0.540000, 0.890000, 0.630000), glm::vec3(0.316228, 0.316228, 0.316228), 0.10000000 * 128 };
    const Material RUBY      { glm::vec3(0.174500, 0.011750, 0.011750), glm::vec3(0.614240, 0.041360, 0.041360), glm::vec3(0.727811, 0.626959, 0.626959), 0.60000000 * 128 };
    const Material PEARL     { glm::vec3(0.250000, 0.207250, 0.207250), glm::vec3(1.000000, 0.829000, 0.829000), glm::vec3(0.296648, 0.296648, 0.296648), 0.08800000 * 128 };
    const Material BRASS     { glm::vec3(0.329412, 0.223529, 0.027451), glm::vec3(0.780392, 0.568627, 0.113725), glm::vec3(0.992157, 0.941176, 0.807843), 0.21794872 * 128 };
    const Material SILVER    { glm::vec3(0.192250, 0.192250, 0.192250), glm::vec3(0.507540, 0.507540, 0.507540), glm::vec3(0.508273, 0.508273, 0.508273), 0.40000000 * 128 };
    const Material BRONZE    { glm::vec3(0.212500, 0.127500, 0.054000), glm::vec3(0.714000, 0.428400, 0.181440), glm::vec3(0.393548, 0.271906, 0.166721), 0.20000000 * 128 };
    const Material CHROME    { glm::vec3(0.250000, 0.250000, 0.250000), glm::vec3(0.400000, 0.400000, 0.400000), glm::vec3(0.774597, 0.774597, 0.774597), 0.60000000 * 128 };
    const Material COPPER    { glm::vec3(0.191250, 0.073500, 0.022500), glm::vec3(0.703800, 0.270480, 0.082800), glm::vec3(0.256777, 0.137622, 0.086014), 0.10000000 * 128 };
    const Material EMERALD   { glm::vec3(0.021500, 0.174500, 0.021500), glm::vec3(0.075680, 0.614240, 0.075680), glm::vec3(0.633000, 0.727811, 0.633000), 0.60000000 * 128 };
    const Material OBSIDIAN  { glm::vec3(0.053750, 0.050000, 0.066250), glm::vec3(0.182750, 0.170000, 0.225250), glm::vec3(0.332741, 0.328634, 0.346435), 0.30000000 * 128 };
    const Material TURQUOISE { glm::vec3(0.100000, 0.187250, 0.174500), glm::vec3(0.396000, 0.741510, 0.691020), glm::vec3(0.297254, 0.308290, 0.306678), 0.10000000 * 128 };

    const Material RED_PLASTIC    { glm::vec3(0.0, 0.0, 0.00), glm::vec3(0.50, 0.00000000, 0.00000000), glm::vec3(0.70000000, 0.60000000, 0.60000000), 0.25 * 128 };
    const Material CYAN_PLASTIC   { glm::vec3(0.0, 0.1, 0.06), glm::vec3(0.00, 0.50980392, 0.50980392), glm::vec3(0.50196078, 0.50196078, 0.50196078), 0.25 * 128 };
    const Material BLACK_PLASTIC  { glm::vec3(0.0, 0.0, 0.00), glm::vec3(0.01, 0.01000000, 0.01000000), glm::vec3(0.50000000, 0.50000000, 0.50000000), 0.25 * 128 };
    const Material GREEN_PLASTIC  { glm::vec3(0.0, 0.0, 0.00), glm::vec3(0.10, 0.35000000, 0.10000000), glm::vec3(0.45000000, 0.55000000, 0.45000000), 0.25 * 128 };
    const Material WHITE_PLASTIC  { glm::vec3(0.0, 0.0, 0.00), glm::vec3(0.55, 0.55000000, 0.55000000), glm::vec3(0.70000000, 0.70000000, 0.70000000), 0.25 * 128 };
    const Material YELLOW_PLASTIC { glm::vec3(0.0, 0.0, 0.00), glm::vec3(0.50, 0.50000000, 0.00000000), glm::vec3(0.60000000, 0.60000000, 0.50000000), 0.25 * 128 };

    const Material RED_RUBBER    { glm::vec3(0.05, 0.00, 0.00), glm::vec3(0.50, 0.40, 0.40), glm::vec3(0.70, 0.04, 0.04), 0.078125 * 128 };
    const Material CYAN_RUBBER   { glm::vec3(0.00, 0.05, 0.05), glm::vec3(0.40, 0.50, 0.50), glm::vec3(0.04, 0.70, 0.70), 0.078125 * 128 };
    const Material BLACK_RUBBER  { glm::vec3(0.02, 0.02, 0.02), glm::vec3(0.01, 0.01, 0.01), glm::vec3(0.40, 0.40, 0.40), 0.078125 * 128 };
    const Material GREEN_RUBBER  { glm::vec3(0.00, 0.05, 0.00), glm::vec3(0.40, 0.50, 0.40), glm::vec3(0.04, 0.70, 0.04), 0.078125 * 128 };
    const Material WHITE_RUBBER  { glm::vec3(0.05, 0.05, 0.05), glm::vec3(0.50, 0.50, 0.50), glm::vec3(0.70, 0.70, 0.70), 0.078125 * 128 };
    const Material YELLOW_RUBBER { glm::vec3(0.05, 0.05, 0.00), glm::vec3(0.50, 0.50, 0.40), glm::vec3(0.70, 0.70, 0.04), 0.078125 * 128 };
};
