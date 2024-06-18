# Computer_Graphics_Lab
这是用C++实现的一些计算机图形学的实验，基于OpenGL

## OpenGL编程基本模式
该实验的实现主要是圆形和长方形的实现。对于圆形，我们使用一个正n边形来逼近，通过一个函数genCircle()来生成多边形的几个顶点，并依次绘制三角形，如下所示：
```cpp
/*
	@brief 生成 圆
	@param cx				圆心的 横坐标
	@param cy				圆心的 纵坐标
	@param r				圆的 半径
	@param numSegments		切分的三角形个数
	@return float*			顶点数组
*/
float* genCircle(float cx, float cy, float r, int numSegments)
```
每个三角形都以圆心和内接正n边形的两个相邻顶点为顶点。

我们使用两个三角形简单地拼接来实现矩形。值得一提的是，我们在生成矩形顶点的函数中，添加了一个参数theta（如下所示），代表矩形的短边与x轴正方向的夹角，我们使用这个参数来控制矩形的旋转。在后续三维图形绘制的实验中，我们将全部使用变换rotate来实现这一点。
```cpp
/*
	@brief 生成 矩形
	@param p_x			顶点的 横坐标
	@param p_y			顶点的 纵坐标
	@param height		矩形的 高
	@param width		矩形的 宽
	@param theta		顶点出发短边与 x轴正方向 的夹角
*/
float* genRectangle(float p_x, float p_y, float height, float width, float theta)
```
接下来，我们在主程序中设置了相应的VAO、VBO，手动计算了各个图形的坐标信息（圆心、半径等），将数据装入并送给GPU，然后把VBO关联给顶点着色器，最后绘制图形。

## 三维模型构建
我们共有两个基本的三维模型：圆柱体和球体。

### 球体
们将球体按经线和纬线两个方向切分，将交点处作为顶点，然后绘制多个矩形（两个三角形拼接）来实现球体，示意图如下所示。为了减少资源使用量，我们使用索引方式复用顶点，因此除了顶点数组，还有一个索引数组，用于在绘制时索引到正确的顶点。当精度为prec时，球体共有(prec+1)(prec+1)个顶点，prec\*prec\*6个索引。我们封装的球体对象是一个球心位于圆点，半径为1的球体，这样做的好处是变换容易、计算顶点也容易。

下面的代码是每轮迭代中顶点坐标的计算。
```cpp
float y = (float)cos(toRadians(180.0f - i * 180.0f / prec));
float x = -(float)cos(toRadians(j*360.0f / prec))*(float)abs(cos(asin(y)));
float z = (float)sin(toRadians(j*360.0f / (float)(prec)))*(float)abs(cos(asin(y)));
```

顶点数组下标和索引的关系如下所示：
```cpp
	indices[6 * (i*prec + j) + 0] = i*(prec + 1) + j;
	indices[6 * (i*prec + j) + 1] = i*(prec + 1) + j + 1;
	indices[6 * (i*prec + j) + 2] = (i + 1)*(prec + 1) + j;
	indices[6 * (i*prec + j) + 3] = i*(prec + 1) + j + 1;
	indices[6 * (i*prec + j) + 4] = (i + 1)*(prec + 1) + j + 1;
	indices[6 * (i*prec + j) + 5] = (i + 1)*(prec + 1) + j;
```
### 圆柱体
我们采用了上下底面两个圆的圆周上的点作为顶点来绘制圆柱体。我们在实验一中以圆心为中心绘制了一系列三角形，而在这个实验中，我们将圆周作n等分，以一个顶点为中心，每次与两个相邻顶点构成一个三角形并绘制两个圆形。对于圆柱体的侧面，我们将其看作一系列的矩形，矩形的四个顶点就是上下底面相对的顶点。因此，精度为prec的情况下，顶点数是2\*(prec+1)，索引数为prec\*6+2(prec-2)\*3。
```cpp
	float x = (float)cos(2 * PI * i / prec);
	float y = (float)sin(2 * PI * i / prec);
	float z = 0.5;
```
这是计算上底面顶点坐标的代码（一轮循环）。我们封装的圆柱体对象是一个底面半径为1，高为1，中轴线为z轴的圆柱体。类似地，我们也实现了映射的数组。

在绘制时，我们使用glm::translate()、glm::rotate()和glm::scale()三个函数，分别实现平移、旋转、放缩。通过变换，我们将基本图形组装成我们想要的模型。此外，由于需要绘制三维图形，我们使用glEnable(GL_DEPTH_TEST)启用深度测试，从而让三维图形可以正确渲染。

## 交互
交互主要由各类回调函数实现。按功能来分，有以下几个部分：
### 视角转动
我们主要实现了通过移动鼠标来改变视角，这是用指针位置回调函数实现的，函数头如下所示。
```cpp
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
```

在实现中，我们通过计算两次调用时鼠标的位置偏移量，计算俯仰角和偏航角，并用这两个数据更新视图矩阵。为了避免万向锁问题，还需要对俯仰角作出限制，我们将其限制在-89°至89°之间。

为了让程序的逻辑更加清晰，我们另外定义了视图矩阵view，使用glm::lookAt()函数将更新后的数据传入顶点着色器。在视角转动这个部分，主要调整的是cameraFront，代表相机的朝向。

### 摄像机移动
用户可以通过键盘来实现摄像机的移动，交换按键的设计如下：
```plaintext
向前 – W
向后 – S
向左 – A
向右 – D
向上 – Space
向下 – Left Shift
```

值得注意的是，我们没有使用按键回调函数实现键盘交互，这是因为它使用起来会较为卡顿，因此我们将其改到主函数的循环中实现，如下所示。
```cpp
	while (!glfwWindowShouldClose(window)) {
		processInput(window);// 处理视角平移
		// other code
	}
```

### 缩放
我们通过鼠标滚轮来改变视野，因此使用了滚轮回调函数实现这个功能，函数声明如下所示。
```cpp
/**
 * @brief 滚轮回调函数
 * @param yoffset		竖直方向滚动的大小
*/
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
```
我们通过改变视场（Field of View），来改变视野，并在每次改变时使用glm::perspective()更新透视投影矩阵。由于变换范围太大会导致整个图形呈现倒像，我们还需要限制视野变换的范围。

### 释放鼠标指针
由于我们实现的是第一人称视角，窗口会隐藏指针并将指针锁定在窗口中央，因此我们需要提供一个按键（设计为左alt）来释放，这里用到按键回调函数。在释放后，我们还需要取消上面所有视角变换的回调函数，防止误操作。按键回调函数的声明如下所示。
```cpp
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
```
相应地，还需要有一个函数重新启用所有的视角变换函数，并将鼠标指针固定在窗口中央并隐藏，从而回到操控模式，这个功能是使用鼠标按键回调函数实现的，其函数声明如下所示。
```cpp
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
```
### 材质
我们首先需要实现每个基本图形的法向量计算。对于球体，顶点的法向量是由圆心指向它的向量；对于圆柱体，法向量是它所在底面的外法向量和它所处的侧面的切平面的法向量的平均值，如下所示。
```cpp
normals[i] = glm::vec3(x/2.0f, y/2.0f, 0.5f);
```
在display方法中，基于设定好的光源位置，我们将包含ADS在内的一系列数据传递给着色器（我们使用了Utils中预先定义好的物体表面材质）。在片元着色器中，我们计算了明暗信息，并将结果与纹理相乘（纹理部分会详细表述纹理相关的实现），这里也可以用相加来实现，但加法实现的明暗效果整体偏亮。需要注意的是，在display方法中，我们需要计算变换矩阵的逆转置矩阵（如下所示），它被用于对法线向量进行变换和插值，以确保在非均匀缩放和非正交变换下保持法线的方向性。
```cpp
invTrMat = glm::transpose(glm::inverse(mvMat));
```
### 光源可视化与交互
我们用一个纯白色小球代表光源。由于我们不希望光源受到环境光等因素的影响，我们使用一套单独的着色器来渲染光源小球。此外，我们使用键盘实现光源位置的改变，这和相机平移的原理是相同的。
### 纹理映射
首先，我们实现了一个天空盒。它是一个固定大小的正方体，跟随相机旋转（通过view矩阵实现）。我们使用Utils::loadCubeMap()加载天空盒六个面的纹理，使用单独的顶点着色器和片元着色器渲染天空盒。在顶点着色器中，使用samplerCube对象处理立方体贴图，同时需要从视图矩阵中删除了跟随相机的平移，否则背景会跟随视角移动。删除后，只保留了天空盒随相机的旋转。删除平移的代码如下所示。
```cpp
mat4 v3_matrix = mat4(mat3(view_matrix));
```
在绘制天空盒时，需要最先绘制并禁用深度测试，使其不再渲染出远近关系，在后续渲染其他对象时重新启用。

之后，我们实现了球体和圆柱体上的纹理映射。对于球体，将纹理坐标按精度等分，直接映射到对应顶点坐标(x, y, z)，如下所示：
```cpp
	vertices[i*(prec + 1) + j] = glm::vec3(x, y, z);
	texCoords[i*(prec + 1) + j] = glm::vec2(((float)j / prec), ((float)i / prec));
```

对于圆柱体，我们直接将纹理的1/3处映射到上底面的顶点，将2/3处映射到下底面的顶点，如下所示（上底面）：
```cpp
	vertices[i] = glm::vec3(x, y, z);
	texCoords[i] = glm::vec2((float)i / prec, 1.0f/3.0f);
```
我们使用Utils::loadTexture()加载纹理，在着色器中使用texture(samp,tc)来应用纹理。

最后，我们对球体应用了凹凸映射。我们从网上下载了法线贴图，并计算了球体的tangent，如下所示：
```cpp
	if (((x == 0) && (y == 1) && (z == 0)) || ((x == 0) && (y == -1) && (z == 0))) {
		tangents[i*(prec + 1) + j] = glm::vec3(0.0f, 0.0f, -1.0f);
	}
	else {
		tangents[i*(prec + 1) + j] = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(x, y, z));
	}

```
对于上下两个顶点，其法向量就是y轴正（或负）向，直接取切平面上一个向量(0,0,-1)，对于其他点，取它的法向量和y轴正向叉乘的值。

随后，在片元着色器中计算TBN矩阵，之后从法线贴图的RGB值恢复垂直偏移（先乘2后减1），将其应用后就可以得到用于实现凹凸映射的法向量。

