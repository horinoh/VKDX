// TriangleVK.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TriangleVK.h"

#pragma region Code
VK* Inst = nullptr;
#pragma endregion

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TRIANGLEVK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TRIANGLEVK));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRIANGLEVK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TRIANGLEVK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
#pragma region Code
	case WM_CREATE:
		if (nullptr == Inst) {
			Inst = new TriangleVK();
		}
		if (nullptr != Inst) {
			try {
				Inst->OnCreate(hWnd, hInst);
			}
			catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}
		break;
	case WM_TIMER:
		if (nullptr != Inst) {
			Inst->OnTimer(hWnd, hInst);
		}
		break;
	case WM_SIZE:
		if (nullptr != Inst) {
			Inst->OnSize(hWnd, hInst);
		}
		break;
	case WM_KEYDOWN:
		if (VK_ESCAPE == wParam) {
			SendMessage(hWnd, WM_DESTROY, 0, 0);
		}
		break;
#pragma endregion
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
#pragma region Code
			if (nullptr != Inst) {
				Inst->OnPaint(hWnd, hInst);
			}
#pragma endregion
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
#pragma region Code
		if (nullptr != Inst) {
			Inst->OnDestroy(hWnd, hInst);
		}
		SAFE_DELETE(Inst);
#pragma endregion
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

#pragma region Code
void TriangleVK::CreateVertexBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
	const std::vector<Vertex> Vertices = {
		{ { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	};
	const auto Stride = sizeof(Vertices[0]);
	const auto Size = static_cast<VkDeviceSize>(Stride * Vertices.size());

	VkBuffer Buffer_Upload;
	VkDeviceMemory DeviceMemory_Upload;
	{
#pragma region Upload
		const VkBufferCreateInfo BufferCreateInfo_Upload = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		Size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo_Upload, nullptr, &Buffer_Upload));
		{
			VkMemoryRequirements MemoryRequirements;
			vkGetBufferMemoryRequirements(Device, Buffer_Upload, &MemoryRequirements);
			const VkMemoryAllocateInfo MemoryAllocateInfo = {
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				MemoryRequirements.size,
				GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			};
			VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &DeviceMemory_Upload));
			void *Data;
			VERIFY_SUCCEEDED(vkMapMemory(Device, DeviceMemory_Upload, 0, /*MemoryAllocateInfo.allocationSize*/Size, 0, &Data)); {
				memcpy(Data, Vertices.data(), Size);
			} vkUnmapMemory(Device, DeviceMemory_Upload);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer_Upload, DeviceMemory_Upload, 0));
		}
#pragma endregion

		{
			const VkBufferCreateInfo BufferCreateInfo = {
				VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				nullptr,
				0,
				Size,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				0, nullptr
			};
			VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &VertexBuffer));

			VkMemoryRequirements MemoryRequirements;
			vkGetBufferMemoryRequirements(Device, VertexBuffer, &MemoryRequirements);
			const VkMemoryAllocateInfo MemoryAllocateInfo = {
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				MemoryRequirements.size,
				GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			};
			VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &VertexDeviceMemory));
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, VertexBuffer, VertexDeviceMemory, 0));
		}
		{
			const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				nullptr,
				CommandPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1
			};
			VkCommandBuffer CopyCommandBuffer;
			VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CopyCommandBuffer)); {
				const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
					VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					nullptr,
					0,
					nullptr
				};
				VERIFY_SUCCEEDED(vkBeginCommandBuffer(CopyCommandBuffer, &CommandBufferBeginInfo)); {
					const VkBufferCopy BufferCopy = {
						0,
						0,
						Size
					};
					vkCmdCopyBuffer(CopyCommandBuffer, Buffer_Upload, VertexBuffer, 1, &BufferCopy);
				} VERIFY_SUCCEEDED(vkEndCommandBuffer(CopyCommandBuffer));

				const VkSubmitInfo SubmitInfo = {
					VK_STRUCTURE_TYPE_SUBMIT_INFO,
					nullptr,
					0, nullptr,
					nullptr,
					1, &CopyCommandBuffer,
					0, nullptr
				};
				VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
				VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
			} vkFreeCommandBuffers(Device, CommandPool, 1, &CopyCommandBuffer);
		}
	}
	vkDestroyBuffer(Device, Buffer_Upload, nullptr);
	vkFreeMemory(Device, DeviceMemory_Upload, nullptr);

#ifdef _DEBUG
	std::cout << "CreateVertexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}

void TriangleVK::CreateIndexBuffer(const VkCommandPool CommandPool, const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties)
{
	const std::vector<uint32_t> Indices = { 0, 1, 2 };

	//!< vkCmdDrawIndexed() が引数に取るので覚えておく必要がある
	IndexCount = static_cast<uint32_t>(Indices.size()); 
	const auto Size = static_cast<VkDeviceSize>(sizeof(Indices[0]) * IndexCount);
	
	VkBuffer Buffer_Upload;
	VkDeviceMemory DeviceMemory_Upload;
	{
#pragma region Upload
		const VkBufferCreateInfo BufferCreateInfo = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			Size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0, nullptr
		};
		VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &Buffer_Upload));
		{
			VkMemoryRequirements MemoryRequirements;
			vkGetBufferMemoryRequirements(Device, Buffer_Upload, &MemoryRequirements);
			const VkMemoryAllocateInfo MemoryAllocateInfo = {
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				MemoryRequirements.size,
				GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			};
			VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &DeviceMemory_Upload));
			void *Data;
			VERIFY_SUCCEEDED(vkMapMemory(Device, DeviceMemory_Upload, 0, /*MemoryAllocateInfo.allocationSize*/Size, 0, &Data)); {
				memcpy(Data, Indices.data(), Size);
			} vkUnmapMemory(Device, DeviceMemory_Upload);
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, Buffer_Upload, DeviceMemory_Upload, 0));
		}
#pragma endregion

		{
			const VkBufferCreateInfo BufferCreateInfo = {
				VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				nullptr,
				0,
				Size,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				0, nullptr
			};
			VERIFY_SUCCEEDED(vkCreateBuffer(Device, &BufferCreateInfo, nullptr, &IndexBuffer));

			VkMemoryRequirements MemoryRequirements;
			vkGetBufferMemoryRequirements(Device, IndexBuffer, &MemoryRequirements);

			const VkMemoryAllocateInfo MemoryAllocateInfo = {
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				MemoryRequirements.size,
				GetMemoryType(PhysicalDeviceMemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			};
			VERIFY_SUCCEEDED(vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &IndexDeviceMemory));
			VERIFY_SUCCEEDED(vkBindBufferMemory(Device, IndexBuffer, IndexDeviceMemory, 0));

			const VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				nullptr,
				CommandPools[0],
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1
			};
			VkCommandBuffer CopyCommandBuffer;
			VERIFY_SUCCEEDED(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CopyCommandBuffer)); {
				const VkCommandBufferBeginInfo CommandBufferBeginInfo = {
					VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					nullptr,
					0,
					nullptr
				};
				VERIFY_SUCCEEDED(vkBeginCommandBuffer(CopyCommandBuffer, &CommandBufferBeginInfo)); {
					const VkBufferCopy BufferCopy = {
						0,
						0,
						Size
					};
					vkCmdCopyBuffer(CopyCommandBuffer, Buffer_Upload, IndexBuffer, 1, &BufferCopy);
				} VERIFY_SUCCEEDED(vkEndCommandBuffer(CopyCommandBuffer));

				const VkSubmitInfo SubmitInfo = {
					VK_STRUCTURE_TYPE_SUBMIT_INFO,
					nullptr,
					0, nullptr,
					nullptr,
					1, &CopyCommandBuffer,
					0, nullptr
				};
				VERIFY_SUCCEEDED(vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE));
				VERIFY_SUCCEEDED(vkQueueWaitIdle(Queue));
			} vkFreeCommandBuffers(Device, CommandPools[0], 1, &CopyCommandBuffer);
		}
	}
	vkDestroyBuffer(Device, Buffer_Upload, nullptr);
	vkFreeMemory(Device, DeviceMemory_Upload, nullptr);

#ifdef _DEBUG
	std::cout << "CreateIndexBuffer" << COUT_OK << std::endl << std::endl;
#endif
}
void TriangleVK::PopulateCommandBuffer(const VkCommandBuffer CommandBuffer)
{
	Super::PopulateCommandBuffer(CommandBuffer);

	//vkCmdSetViewport(CommandBuffer, 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
	//vkCmdSetScissor(CommandBuffer, 0, static_cast<uint32_t>(ScissorRects.size()), ScissorRects.data());

	//TODO
#if 0
	assert(!Framebuffers.empty());
	assert(!ScissorRects.empty());

	const std::vector<VkClearValue> ClearValues = {
		{ 0.5f, 0.5f, 1.0f, 1.0f }, { 1.0f, 0 }
	};
	const VkRenderPassBeginInfo RenderPassBeginInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		nullptr,
		RenderPass,
		Framebuffers[SwapchainImageIndex],
		ScissorRects[0],
		static_cast<uint32_t>(ClearValues.size()), ClearValues.data()
	};
	vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); {
		//vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, nullptr);

		//!< トポロジは Pipeline - VkPipelineInputAssemblyStateCreateInfo で指定しているのでパイプラインをバインド
		vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

		const VkDeviceSize Offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &VertexBuffer, Offsets);
		vkCmdBindIndexBuffer(CommandBuffer, IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		
		vkCmdDrawIndexed(CommandBuffer, IndexCount, 1, 0, 0, 0);

	} vkCmdEndRenderPass(CommandBuffer);
#endif
}
#pragma endregion