///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	// destroy the created OpenGL textures
	DestroyGLTextures();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method loads all texture image files into memory,
 *  configures OpenGL texture parameters, generates mipmaps,
 *  and binds them to texture units.
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// Texture 1: Natural rustic wood texture for the desk surface
	bReturn = CreateGLTexture(
		"../../Utilities/textures/rusticwood.jpg",
		"wood");

	// Texture 2: Clean white ceramic porcelain texture for the coffee mug body & handle
	bReturn = CreateGLTexture(
		"../../Utilities/textures/drywall.jpg",
		"ceramic");

	// Texture 3: Rich coffee surface texture for the liquid cylinder inside the mug
	bReturn = CreateGLTexture(
		"../../Utilities/textures/breadcrust.jpg",
		"coffee");

	// Texture 4: Circular brushed gold metallic texture for eyeglasses frames & bridge
	bReturn = CreateGLTexture(
		"../../Utilities/textures/circular-brushed-gold-texture.jpg",
		"gold");

	// Texture 5: Dark composite matte metal texture for eyeglasses temple arms
	bReturn = CreateGLTexture(
		"../../Utilities/textures/knife_handle.jpg",
		"matte_metal");

	// Texture 6: Brushed stainless steel/aluminum texture for keyboard base shell
	bReturn = CreateGLTexture(
		"../../Utilities/textures/stainless.jpg",
		"aluminum");

	// Texture 7: Dark matte textured surface for keyboard keybed cluster block
	bReturn = CreateGLTexture(
		"../../Utilities/textures/backdrop.jpg",
		"keybed");

	// Bind all loaded textures to the available OpenGL texture slots
	BindGLTextures();
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the basic shape meshes into memory to support 3D rendering.
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// Load all scene textures
	LoadSceneTextures();

	// Load all 3D basic shape meshes required to construct the objects
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTorusMesh(0.25f);
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPrismMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method renders the complete 3D scene by calling
 *  modularized helper routines for each complex 3D object.
 ***********************************************************/
void SceneManager::RenderScene()
{
	// Render the ground desktop surface
	RenderDeskSurface();

	// Render Complex Object 1: Coffee Cup (Cylinder body + Torus handle + Cylinder coffee)
	RenderCoffeeCup();

	// Render Complex Object 2: Eyeglasses (2 Torus frames + Cylinder bridge + 2 Box temple arms)
	RenderEyeglasses();

	// Render Complex Object 3: Keyboard (Flattened Box base + Box key cluster)
	RenderKeyboard();
}

/***********************************************************
 *  RenderDeskSurface()
 *
 *  Renders the main desk surface using a scaled Plane primitive.
 ***********************************************************/
void SceneManager::RenderDeskSurface()
{
	// Transformations: Scale first, then Rotate, then Translate
	glm::vec3 scaleXYZ = glm::vec3(20.0f, 1.0f, 14.0f);
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply rustic wood texture with complex tiling technique (4x3 repeat across large plane)
	SetShaderTexture("wood");
	SetTextureUVScale(4.0f, 3.0f);

	// Draw ground plane mesh
	m_basicMeshes->DrawPlaneMesh();
}

/***********************************************************
 *  RenderCoffeeCup()
 *
 *  Replicates the coffee cup from the 2D reference image using
 *  a combination of basic 3D shapes:
 *   1. Cylinder (Mug body)
 *   2. Torus (Handle attached to the side)
 *   3. Cylinder (Coffee liquid inside top of mug)
 ***********************************************************/
void SceneManager::RenderCoffeeCup()
{
	// Anchor position for the coffee cup on the left side of the desk
	glm::vec3 cupPosition = glm::vec3(-4.5f, 0.0f, 1.0f);

	// ---------------------------------------------------------
	// 1. MUG BODY (Cylinder)
	// ---------------------------------------------------------
	// Scale: Radius 1.25, Height 2.5
	glm::vec3 scaleXYZ = glm::vec3(1.25f, 2.5f, 1.25f);
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	// Placed flush on desk surface (Y = 0.0f)
	glm::vec3 positionXYZ = cupPosition;

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply ceramic porcelain texture to mug body
	SetShaderTexture("ceramic");
	SetTextureUVScale(1.0f, 1.0f);

	// Draw upright cylinder mesh
	m_basicMeshes->DrawCylinderMesh(true, true, true);

	// ---------------------------------------------------------
	// 2. MUG HANDLE (Torus)
	// ---------------------------------------------------------
	// Scale: Radius 0.75
	scaleXYZ = glm::vec3(0.75f, 0.75f, 0.75f);
	// Rotate 90 degrees around Z axis to orient the handle loop vertically on the side
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	// Attached to the left outer wall of the mug body
	positionXYZ = cupPosition + glm::vec3(-1.4f, 1.25f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Ceramic porcelain texture matching the mug body
	SetShaderTexture("ceramic");
	SetTextureUVScale(1.0f, 1.0f);

	// Draw torus mesh
	m_basicMeshes->DrawTorusMesh();

	// ---------------------------------------------------------
	// 3. COFFEE LIQUID (Cylinder)
	// ---------------------------------------------------------
	// Scale: Radius 1.15 (just inside mug rim), Height 0.06
	scaleXYZ = glm::vec3(1.15f, 0.06f, 1.15f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	// Positioned near top opening inside the mug
	positionXYZ = cupPosition + glm::vec3(0.0f, 2.35f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Rich roasted coffee crema texture
	SetShaderTexture("coffee");
	SetTextureUVScale(1.0f, 1.0f);

	// Draw cylinder mesh
	m_basicMeshes->DrawCylinderMesh(true, true, true);
}

/***********************************************************
 *  RenderEyeglasses()
 *
 *  Replicates the pair of glasses from the 2D reference image
 *  using a combination of basic 3D shapes:
 *   1. Torus (Left lens frame)
 *   2. Torus (Right lens frame)
 *   3. Cylinder (Nose bridge connecting the frames)
 *   4. Box (Left temple arm extending backward)
 *   5. Box (Right temple arm extending backward)
 ***********************************************************/
void SceneManager::RenderEyeglasses()
{
	// Anchor position for glasses on the right foreground of desk
	glm::vec3 glassesCenter = glm::vec3(3.8f, 0.22f, 2.8f);

	// ---------------------------------------------------------
	// 1. LEFT LENS FRAME (Torus) - Gold metallic finish
	// ---------------------------------------------------------
	glm::vec3 scaleXYZ = glm::vec3(0.75f, 0.75f, 0.75f);
	// Rotate 90 degrees on X axis so the frame rests flat on table
	float XrotationDegrees = 90.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ = glassesCenter + glm::vec3(-0.95f, 0.0f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply brushed gold texture
	SetShaderTexture("gold");
	SetTextureUVScale(1.0f, 1.0f);

	m_basicMeshes->DrawTorusMesh();

	// ---------------------------------------------------------
	// 2. RIGHT LENS FRAME (Torus) - Gold metallic finish
	// ---------------------------------------------------------
	scaleXYZ = glm::vec3(0.75f, 0.75f, 0.75f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glassesCenter + glm::vec3(0.95f, 0.0f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply brushed gold texture
	SetShaderTexture("gold");
	SetTextureUVScale(1.0f, 1.0f);

	m_basicMeshes->DrawTorusMesh();

	// ---------------------------------------------------------
	// 3. NOSE BRIDGE (Horizontal Cylinder) - Gold metallic finish
	// ---------------------------------------------------------
	// Scale: Thin radius 0.08, length 1.2
	scaleXYZ = glm::vec3(0.08f, 1.2f, 0.08f);
	// Rotate 90 degrees around Z axis to lie horizontal between lens frames
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glassesCenter;

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply brushed gold texture
	SetShaderTexture("gold");
	SetTextureUVScale(1.0f, 1.0f);

	m_basicMeshes->DrawCylinderMesh(true, true, true);

	// ---------------------------------------------------------
	// 4. LEFT TEMPLE ARM (Box) - Matte dark metal texture
	// ---------------------------------------------------------
	// Scale: Thin width 0.1, height 0.1, length 2.2
	scaleXYZ = glm::vec3(0.1f, 0.1f, 2.2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -12.0f; // Slight outward angle
	ZrotationDegrees = 0.0f;
	positionXYZ = glassesCenter + glm::vec3(-1.8f, 0.0f, -1.1f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply matte dark metal texture
	SetShaderTexture("matte_metal");
	SetTextureUVScale(1.0f, 1.0f);

	m_basicMeshes->DrawBoxMesh();

	// ---------------------------------------------------------
	// 5. RIGHT TEMPLE ARM (Box) - Matte dark metal texture
	// ---------------------------------------------------------
	scaleXYZ = glm::vec3(0.1f, 0.1f, 2.2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 12.0f; // Slight outward angle
	ZrotationDegrees = 0.0f;
	positionXYZ = glassesCenter + glm::vec3(1.8f, 0.0f, -1.1f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply matte dark metal texture
	SetShaderTexture("matte_metal");
	SetTextureUVScale(1.0f, 1.0f);

	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  RenderKeyboard()
 *
 *  Replicates the computer keyboard from the 2D reference image
 *  using basic 3D shapes:
 *   1. Box (Flattened keyboard base shell)
 *   2. Box (Elevated keybed cluster block)
 ***********************************************************/
void SceneManager::RenderKeyboard()
{
	// Anchor position for keyboard centered in upper portion of desk
	glm::vec3 kbPosition = glm::vec3(0.0f, 0.0f, -1.8f);

	// ---------------------------------------------------------
	// 1. KEYBOARD BASE SHELL (Box) - Brushed aluminum texture
	// ---------------------------------------------------------
	// Scale: Width 9.0, Height 0.3, Depth 3.6
	glm::vec3 scaleXYZ = glm::vec3(9.0f, 0.3f, 3.6f);
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	// Position centered, resting on desk surface (Y = height / 2 = 0.15f)
	glm::vec3 positionXYZ = kbPosition + glm::vec3(0.0f, 0.15f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply brushed aluminum texture
	SetShaderTexture("aluminum");
	SetTextureUVScale(1.0f, 1.0f);

	m_basicMeshes->DrawBoxMesh();

	// ---------------------------------------------------------
	// 2. KEYBED CLUSTER BLOCK (Box) - Textured keybed with UV scaling
	// ---------------------------------------------------------
	// Scale: Width 8.3, Height 0.18, Depth 3.0
	scaleXYZ = glm::vec3(8.3f, 0.18f, 3.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	// Positioned elevated on top of keyboard base
	positionXYZ = kbPosition + glm::vec3(0.0f, 0.34f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Apply dark textured material with 2.0x UV horizontal scale for key matrix
	SetShaderTexture("keybed");
	SetTextureUVScale(2.0f, 1.0f);

	m_basicMeshes->DrawBoxMesh();
}

