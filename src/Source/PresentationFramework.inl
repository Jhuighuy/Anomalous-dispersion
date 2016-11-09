// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#pragma once

namespace Presentation2
{

	// -----------------------
	template<typename TGraphicsWidget, typename... TArgs>
	ADAPI std::shared_ptr<TGraphicsWidget> Window::Graphics(Rect const& rect, TArgs&&... args) const
	{
		static_assert(std::is_base_of_v<GraphicsWidget, TGraphicsWidget>, "Invalid base type.");
		assert(rect.Width != 0 && rect.Height != 0);

		/* Initializing the Direct3D 9 driver. */
		auto static const direct3D = Utils::RuntimeCheck(Direct3DCreate9(D3D_SDK_VERSION));

		/* Creating a static widget to render into it. */
		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE
			, rect.X, rect.Y, rect.Width, rect.Height, m_Handle, nullptr, nullptr, nullptr));

		/* Creating the device. */
		D3DPRESENT_PARAMETERS static presentParameters = {};
		presentParameters.Windowed = TRUE;
		presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
		presentParameters.hDeviceWindow = handle;
		presentParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
		presentParameters.BackBufferWidth = rect.Width;
		presentParameters.BackBufferHeight = rect.Height;
		presentParameters.EnableAutoDepthStencil = TRUE;
		presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
		presentParameters.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;

		IDirect3DDevice9* device = nullptr;
		for (DWORD const vertexProcessing : { D3DCREATE_MIXED_VERTEXPROCESSING, D3DCREATE_HARDWARE_VERTEXPROCESSING, D3DCREATE_SOFTWARE_VERTEXPROCESSING })
		{
			/* Trying to create a hardware vertex processing first and only then software. */
			if (SUCCEEDED(direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle
				, D3DCREATE_MULTITHREADED | vertexProcessing, &presentParameters, &device)))
			{
				break;
			}
		}
		return std::make_shared<TGraphicsWidget>(handle, Utils::RuntimeCheck(device), std::forward<TArgs>(args)...);
	}

}	// Presentation2
