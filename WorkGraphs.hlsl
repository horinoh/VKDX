//!< �ŐV�� dxc ���_�E�����[�h���ĉ�
//!< https://github.com/microsoft/DirectXShaderCompiler/releases
//!< dxc.exe ���g�p���ăR���p�C��
//!< $dxc.exe -E main -T lib_6_8 -Fo XXX.sco XXX.hlsl

struct EntryRecord
{
    uint GridSize : SV_DispatchGrid;
    uint RecordIndex;
};

struct SecondNodeInput
{
    uint EntryRecordIndex;
    uint IncrementValue;
};

//!< Work Graphs �ł� "node"
[Shader("node")]
//!< "broadcasting", "coalescing", "thread" ������
[NodeLaunch("broadcasting")]
//!< DispatchGraph �����̍ő�l
[NodeMaxDispatchGrid(16, 1, 1)]
//!< �X���b�h�O���[�v�̃X���b�h��
[NumThreads(2, 1, 1)]
void main(DispatchNodeInputRecord<EntryRecord> InputData, [MaxRecords(2)] NodeOutput<SecondNodeInput> SecondNode, uint ThreadIndex : SV_GroupIndex, uint DispatchThreadID : SV_DispatchThreadID)
{
    //!< �o�͗p�̊m�ہA�X���b�h�O���[�v�P�ʂŎ��s�����
    GroupNodeOutputRecords<SecondNodeInput> Out = SecondNode.GetGroupNodeOutputRecords(2);

    Out[ThreadIndex].EntryRecordIndex = InputData.Get().RecordIndex;
    Out[ThreadIndex].IncrementValue = DispatchThreadID * 2 + ThreadIndex + 1;

    //!< �X���b�h�O���[�v�P�ʂŎ��s�����
    Out.OutputComplete();
}
