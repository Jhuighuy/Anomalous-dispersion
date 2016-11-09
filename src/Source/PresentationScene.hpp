// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#pragma once
#include "PresentationEngine.hpp"

namespace Presentation2
{
	struct PrismController
	{
	};

	struct PresentationScene final : public Scene
	{
	private:


	public:
		ADINT explicit PresentationScene(IDirect3DDevice9* const device)
			: Scene(device)
		{
			{	/* Setting up the camera. */
				auto const camera = OrbitalCamera();
				camera->Rotation = { 0.0, -dxm::radians(90.0f), 0.0 };
				camera->RotationCenter = { 0.0f, 1.2f, 2.0f };
				camera->CenterOffset = { 0.0f, 0.0f, -1.8f };
			}

			auto const roomMesh = std::make_shared<TriangleMesh>(device, L"../gfx/room.obj");

			auto const room = TriangleMeshRenderer(roomMesh, L"../gfx/roomLightMap.png");
			room->Position.z = 2.0f;
			room->Rotation.y = F_PI;
		}

	};	// struct PresentationScene

}	// namespace Presentation2
