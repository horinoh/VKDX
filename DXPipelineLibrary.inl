#ifdef USE_PIPELINE_SERIALIZE
class PipelineLibrarySerializer
{
public:
	PipelineLibrarySerializer(ID3D12Device* Dev, const std::wstring& Path) : Device(Dev), FilePath(Path) {
#ifdef ALWAYS_REBUILD_PIPELINE
		DeleteFile(data(FilePath));
#endif
		COM_PTR<ID3D12Device1> Device1;
		VERIFY_SUCCEEDED(Device->QueryInterface(COM_PTR_UUIDOF_PUTVOID(Device1)));

		COM_PTR<ID3DBlob> Blob;
		if (SUCCEEDED(D3DReadFileToBlob(data(FilePath), COM_PTR_PUT(Blob))) && Blob->GetBufferSize()) {
			Logf("PipelineLibrarySerializer : Reading PipelineLibrary = %ls\n", data(FilePath));
			//!< �t�@�C�����ǂ߂��ꍇ�� PipelineLinrary �֓ǂݍ��� (If file is read, load to PipplineLibrary)
			VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(Blob->GetBufferPointer(), Blob->GetBufferSize(), COM_PTR_UUIDOF_PUTVOID(PipelineLibrary)));
			IsLoaded = true;
		}
		else {
			Log("PipelineLibrarySerializer : Creating PipelineLibrary\n");
			//!< �t�@�C�����ǂ߂Ȃ������ꍇ�͏������ݗp�ɐV�K�쐬 (If file is not read, create new for write)
			VERIFY_SUCCEEDED(Device1->CreatePipelineLibrary(nullptr, 0, COM_PTR_UUIDOF_PUTVOID(PipelineLibrary)));
		}
	}
	virtual ~PipelineLibrarySerializer() {
		if (!IsLoaded) {
			Logf("PipelineLibrarySerializer : Writing PipelineLibrary = %ls\n", data(FilePath));
			//!< �t�@�C���֏������� (Write to file)
			const auto Size = PipelineLibrary->GetSerializedSize();
			if (Size) {
				COM_PTR<ID3DBlob> Blob;
				VERIFY_SUCCEEDED(D3DCreateBlob(Size, COM_PTR_PUT(Blob)));
				VERIFY_SUCCEEDED(PipelineLibrary->Serialize(Blob->GetBufferPointer(), Size));
				VERIFY_SUCCEEDED(D3DWriteBlobToFile(COM_PTR_GET(Blob), data(FilePath), TRUE));
			}
		}
		LOG_OK();
	}
	ID3D12PipelineLibrary* GetPipelineLibrary() const { return COM_PTR_GET(PipelineLibrary); }
	bool IsLoadSucceeded() const { return IsLoaded; }
private:
	ID3D12Device* Device;
	std::wstring FilePath;
	COM_PTR<ID3D12PipelineLibrary> PipelineLibrary;
	bool IsLoaded = false;
};
#endif //!< USE_PIPELINE_SERIALIZE