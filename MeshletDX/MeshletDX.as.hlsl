struct PAYLOAD_OUT
{
    uint InstanceID;
    uint MeshletDimension;
    uint MeshletIDs[32];
};

groupshared PAYLOAD_OUT Payload;

#define N 5
#define NN (N * N)

//!< ������, LOD, �e�Z���[�V������, ���I�Ȃ��̂ɃA���v���t�B�P�[�V�����V�F�[�_�[���g���A�ÓI�Ȃ��̂Ȃ烁�b�V���V�F�[�_�݂̂ŗǂ�
[numthreads(NN, 1, 1)]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    Payload.InstanceID = GroupID;
    Payload.MeshletDimension = N;

    bool Visible = true;
    //bool Visible = 0 == (GroupThreadID % 2);
    //bool Visible = 1 == (GroupThreadID % 2);
    //bool Visible = (GroupID % 2) == (GroupThreadID % 2);

    //!< Wave���߂̓X���b�h�ԓ��������
    
    if (Visible)
    {
        //!< WavePrefixCountBits() : true ��Ԃ�(���Ⴂ)�X���b�h����Ԃ��A���ʓI�� true ��Ԃ��X���b�h ID ���O�l�߂Ŋi�[����Ă���
        Payload.MeshletIDs[WavePrefixCountBits(Visible)] = GroupThreadID;
    }

    //!< WaveActiveCountBits() : true ��Ԃ��S�X���b�h����Ԃ��A���̕�(���b�V�����b�g��)�������b�V���V�F�[�_���N������(DispatchMesh())
    DispatchMesh(WaveActiveCountBits(Visible), 1, 1, Payload);
}