#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

int main() {
	//�f�o�C�X�𒊏ۉ�����p�C�v���C�����\�z
	rs2::pipeline pipe;

	//�p�C�v���C���̐ݒ�
	rs2::config cfg;

	//�����I�ɃX�g���[����L��������
	//640�~480�̃J���[�X�g���[���摜���ABGR8�t�H�[�}�b�g�A30fps�Ŏ擾����悤�ɐݒ�
	cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);

	//�ݒ��K�p���ăp�C�v���C���X�g���[�~���O���J�n
	pipe.start(cfg);

	while (true) {
		//�X�g���[�����t���[���Z�b�g���擾����܂őҋ@
		rs2::frameset frames = pipe.wait_for_frames();

		//�t���[���Z�b�g����J���[�t���[�����擾
		rs2::frame color = frames.get_color_frame();

		//�J���[�t���[������AOpenCV��Mat���쐬
		cv::Mat frame(cv::Size(640, 480), CV_8UC3, (void*)color.get_data(), cv::Mat::AUTO_STEP);

		//�摜��\��
		cv::imshow("window", frame);

		//q�������ƏI��
		int key = cv::waitKey(1);
		if (key == 113) {
			break;
		}
	}

	//�S�ẴE�B���h�E�����
	cv::destroyAllWindows();

	return 0;
}
