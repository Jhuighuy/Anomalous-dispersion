// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "PresentationEngine.hpp"

#ifdef SUPPORT_LOADING_FROM_FILE
#include <d3dx9.h>
#endif	// ifdef SUPPORT_LOADING_FROM_FILE

namespace Presentation2
{
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Camera setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	ADAPI Camera::Camera(IDirect3DDevice9* const device) 
		: m_Device(device), m_Rect{}
	{
		assert(m_Device != nullptr);

		D3DDEVICE_CREATION_PARAMETERS deviceCreationParameters = {};
		Utils::RuntimeCheckH(m_Device->GetCreationParameters(&deviceCreationParameters));
		Utils::RuntimeCheck(GetWindowRect(deviceCreationParameters.hFocusWindow, &m_Rect));

		Camera::Update();
	}

	// -----------------------
	ADAPI void Camera::Update() const
	{
		auto const projectionMatrix = dxm::perspectiveFovLH<FLOAT>(FieldOfView, GetContextWidth(), GetContextHeight(), NearClippingPlane, FarClippingPlane);
		auto const viewMatrix = dxm::inverse(dxm::translate(Position) * dxm::toMat4(dxm::quat(dxm::vec3(Rotation.x, Rotation.y, Rotation.z))));

		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_PROJECTION, dxm::ptr(projectionMatrix)));
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_VIEW, dxm::ptr(viewMatrix)));
	}

	// -----------------------
	ADAPI void OrbitalCamera::Update(bool const forceUpdate) const
	{
		if (GetAsyncKeyState(VK_LBUTTON) != 0 || forceUpdate)
		{
			POINT mouseCurrentPosition = { };
			GetCursorPos(&mouseCurrentPosition);
			if (PtInRect(&m_Rect, mouseCurrentPosition) || forceUpdate)
			{
				/* Mouse has been moved inside the context rect while left button has been pressed. */
				auto const deltaYaw = forceUpdate ? 0.0f : static_cast<FLOAT>(mouseCurrentPosition.y - m_PrevMousePosition.y) / GetContextWidth();
				auto const deltaPitch = forceUpdate ? 0.0f : static_cast<FLOAT>(mouseCurrentPosition.x - m_PrevMousePosition.x) / GetContextHeight();

				m_CameraRotationYaw += deltaPitch;
				m_CameraRotationPitch = dxm::clamp(m_CameraRotationPitch + deltaYaw, -F_PI / 12.0f, F_PI / 7.5f);

				auto const translation = dxm::translate(glm::vec3(RotationCenter));
				auto const cameraRotation = dxm::yawPitchRoll(m_CameraRotationYaw, m_CameraRotationPitch, 0.0f);
				m_ViewMatrix = dxm::lookAtLH(dxm::vec3(translation * cameraRotation * dxm::vec4(CenterOffset, 1.0f))
					, RotationCenter, dxm::vec3(cameraRotation * dxm::vec4(Up, 1.0f)));
			}
		}
		GetCursorPos(&m_PrevMousePosition);

		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_VIEW, dxm::ptr(m_ViewMatrix)));
		Utils::RuntimeCheckH(m_Device->SetTransform(D3DTS_PROJECTION, dxm::ptr(m_ProjectionMatrix)));
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Textures and Pixel shaders setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
#ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI void LoadOBJ(wchar_t const* const path, TriangleMutableMeshPtr const& mesh, dxm::argb const color)
	{
		assert(path != nullptr);

#if !_DEBUG
		OutputDebugStringA("Loading resource from file in release version.");
#endif	// if !_DEBUG

		std::vector<UINT> vertexIndices, texCoordIndices, normalIndices;
		std::vector<dxm::vec3> vertexBuffer;
		std::vector<dxm::vec2> texCoordBuffer;
		std::vector<dxm::vec3> normalBuffer;

		FILE* file = nullptr;
		Utils::RuntimeCheck(_wfopen_s(&file, path, L"r") == 0);

		/* Reading the file.. */
		for (char lineHeader[16] = {}; fscanf_s(file, "%s", lineHeader, dxm::countof(lineHeader)) != EOF;)
		{
			/* Reading first word of the line.. */
			if (lineHeader[0] == 'v' && lineHeader[1] == '\0')
			{
				/* .. It is a vertex coord. */
				dxm::vec3 vertex;
				Utils::RuntimeCheck(fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z) == 3);
				vertexBuffer.push_back(vertex);
			}
			else if (lineHeader[0] == 'v' && lineHeader[1] == 'n' && lineHeader[2] == '\0')
			{
				/* .. It is a vertex normal. */
				dxm::vec3 normal;
				Utils::RuntimeCheck(fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z) == 3);
				normalBuffer.push_back(normal);
			}
			else if (lineHeader[0] == 'v' && lineHeader[1] == 't' && lineHeader[2] == '\0')
			{
				/* .. It is a texture coord. */
				dxm::dvec3 texCoord;
				Utils::RuntimeCheck(fscanf_s(file, "%lf %lf %lf\n", &texCoord.x, &texCoord.y, &texCoord.z) == 3);
				texCoordBuffer.push_back(glm::vec2(texCoord.x, 1 - texCoord.y));
			}
			else if (lineHeader[0] == 'f' && lineHeader[1] == '\0')
			{
				/* .. It is a face. */
				vertexIndices.resize(vertexIndices.size() + 3);
				texCoordIndices.resize(texCoordIndices.size() + 3);
				normalIndices.resize(normalIndices.size() + 3);
				Utils::RuntimeCheck(fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n"
					, &vertexIndices[vertexIndices.size() - 3], &texCoordIndices[texCoordIndices.size() - 3], &normalIndices[normalIndices.size() - 3]
					, &vertexIndices[vertexIndices.size() - 2], &texCoordIndices[texCoordIndices.size() - 2], &normalIndices[normalIndices.size() - 2]
					, &vertexIndices[vertexIndices.size() - 1], &texCoordIndices[texCoordIndices.size() - 1], &normalIndices[normalIndices.size() - 1]) == 9);
			}
			else
			{
				/* .. It is something strange, just skipping. */
				char static restOfTheLine[256] = {};
				fgets(restOfTheLine, dxm::countof(restOfTheLine), file);
			}
		}

		_fclose_nolock(file);

		/* Unwrapping internal OBJ format.. */
		mesh->BeginUpdateVertices();
		for (auto cnt = 0u; cnt < vertexIndices.size(); ++cnt)
		{
			auto const& vertex = vertexBuffer[vertexIndices[cnt] - 1];
			auto const& texCoord = texCoordBuffer[texCoordIndices[cnt] - 1];
			auto const& normal = normalBuffer[normalIndices[cnt] - 1];

			mesh->AddVertex({ vertex, normal, color, texCoord });
		}
		mesh->EndUpdateVertices();
	}
#endif	// ifdef SUPPORT_LOADING_FROM_FILE

	// -----------------------
#ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI void LoadTexture(IDirect3DDevice9* const device, wchar_t const* const path, IDirect3DTexture9** const texturePtr)
	{
		assert(device != nullptr);
		assert(path != nullptr);
		assert(texturePtr != nullptr);

#if !_DEBUG
		OutputDebugStringA("Loading resource from file in release version.");
#endif	// if !_DEBUG

		Utils::RuntimeCheckH(D3DXCreateTextureFromFileW(device, path, texturePtr));
	}
#endif	// ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI void LoadTexture(IDirect3DDevice9* const device, LoadFromMemory_t, void const* const data, UINT const width, UINT const height, IDirect3DTexture9** const texturePtr)
	{
		assert(device != nullptr);
		assert(data != nullptr);
		assert(width != 0 && height != 0);
		assert(texturePtr != nullptr);

		/* Creating sized texture.. */
		IDirect3DTexture9* texture = nullptr;
		Utils::RuntimeCheckH(device->CreateTexture(width, height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr));
		*texturePtr = texture;

		/* Uploading texture information to the GPU.. */
		IDirect3DSurface9* surface = nullptr;
		Utils::RuntimeCheckH(texture->GetSurfaceLevel(0, &surface));
		D3DLOCKED_RECT surfaceRect = {};
		Utils::RuntimeCheckH(surface->LockRect(&surfaceRect, nullptr, D3DLOCK_DISCARD));
		auto const textureSize = width * height * sizeof(dxm::argb);
		::memcpy_s(surfaceRect.pBits, textureSize, data, textureSize);
		Utils::RuntimeCheckH(surface->UnlockRect());
		surface->Release();

		texture->GenerateMipSubLevels();
	}

	// -----------------------
#ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI void LoadPixelShader(IDirect3DDevice9* const device, wchar_t const* const path, IDirect3DPixelShader9** const pixelShaderPtr)
	{
		assert(device != nullptr);
		assert(path != nullptr);
		assert(pixelShaderPtr != nullptr);

#if !_DEBUG
		OutputDebugStringA("Loading resource from file in release version.");
#endif	// if !_DEBUG

		ID3DXBuffer* pixelShaderCode = nullptr;
		ID3DXBuffer* pixelShaderErrors = nullptr;
		if (FAILED(D3DXCompileShaderFromFileW(path, nullptr, nullptr, "main", "ps_2_0", 0, &pixelShaderCode, &pixelShaderErrors, nullptr)))
		{
			MessageBoxA(nullptr, static_cast<char const*>(pixelShaderErrors->GetBufferPointer()), "Error", MB_OK);
			Utils::RuntimeCheckH(E_FAIL);
		}
		LoadPixelShader(device, LoadFromMemory, pixelShaderCode->GetBufferPointer(), pixelShaderPtr);
	}
#endif	// ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI void LoadPixelShader(IDirect3DDevice9* const device, LoadFromMemory_t, void const* const compiledData, IDirect3DPixelShader9** const pixelShaderPtr)
	{
		assert(device != nullptr);
		assert(compiledData != nullptr);
		assert(pixelShaderPtr != nullptr);

		Utils::RuntimeCheckH(device->CreatePixelShader(static_cast<DWORD const*>(compiledData), pixelShaderPtr));
	}

}	// namespace Presentation2
