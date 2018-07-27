// hand_motion_tracking.cpp : 簡易的な手の動きの検出を行う

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>	
#include <iostream>

using namespace std;

//カスケード分類器と深度情報による手の検出を行う
float detectHandInImage(cv::Mat &image, std::string &cascade_file, rs2::depth_frame &depth) {
	//カスケードファイルの読み込み
	cv::CascadeClassifier cascade;
	cascade.load(cascade_file);
	
	//手の検出
	vector<cv::Rect> hands;
	cascade.detectMultiScale(image, hands, 1.2, 3, 0, cv::Size(30, 30));

	int nearest_hand = 0;
	float nearest_dist = 100.0;

	//検出したオブジェクトの中で最も近いものを判定
	for (int i = 0; i < hands.size(); i++) {
		if (hands.size() != 0 && depth) {
			//検出したオブジェクトの距離を取得
			float dist_to_hand = depth.get_distance(hands[i].x + (hands[i].width / 2), hands[i].y + (hands[i].height / 2));
			if (nearest_dist > dist_to_hand) {
				nearest_dist = dist_to_hand;
				nearest_hand = i;
			}
		}
	}

	if (hands.size() != 0) {
		//一番近いものを手として検出
		//cout << "検出したオブジェクトとの距離 " << nearest_dist << "m \r";
		rectangle(image, cv::Point(hands[nearest_hand].x, hands[nearest_hand].y), 
						cv::Point(hands[nearest_hand].x + hands[nearest_hand].width, hands[nearest_hand].y
						+ hands[nearest_hand].height), cv::Scalar(0, 200, 0), 3, CV_AA);
	}
	
	return nearest_dist;
}

//手の距離の変化からモーションを検出する
int detectHandMoved(std::vector<float> &dists_vec, float dist_to_hand) {
	//距離の情報を追加
	dists_vec.push_back(dist_to_hand);
	//5フレーム分溜まっていなければ処理は終了
	if (dists_vec.size() <= 5) return 0;

	//一番古い要素を削除
	dists_vec.erase(dists_vec.begin());

	//フレームごとの変化量を取得
	int motion_flg = 0;
	for (int i = 0; i < dists_vec.size() - 1; i++) {
		if (dists_vec[i + 1] - dists_vec[i] > 0.015) {
			motion_flg++;
		} else if (dists_vec[i + 1] - dists_vec[i] < -0.015 ) {
			motion_flg--;
		} else {
			return 0;
		}
	}
	//全て一定以上の変化量であればモーションあり
	//保持フレームを空にする
	dists_vec.clear();
	if (motion_flg == 5) {
		cout << "PULL" << endl;
		return 1;
	}
	else if (motion_flg = -5) {
		cout << "PUSH" << endl;
		return 2;
	}
	
	return 0;
}

int main() {
	//手の移動を検出するための動的配列
	static vector<float> dists_vec;

	//カスケードファイルの用意
	string cascadefile = "aGest.xml";
	
	//パイプラインの作成
	rs2::pipeline pipe;
	rs2::config cfg;
	//キャプチャするフレームの設定
	cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);

	//ストリーミングの開始
	pipe.start(cfg);

	while (true) {
		//カメラ映像のキャプチャ
		rs2::frameset frames = pipe.wait_for_frames();

		//デプスフレームをRGBフレームに合わせる
		rs2::align align(RS2_STREAM_COLOR);
		auto aligned_frames = align.process(frames);

		//RGBフレームを取得
		rs2::frame color = frames.get_color_frame();
		//デプスフレームを取得
		rs2::depth_frame depth = aligned_frames.get_depth_frame();

		//キャプチャ画像をフレームに代入
		cv::Mat frame(cv::Size(640, 480), CV_8UC3, (void*)color.get_data(), cv::Mat::AUTO_STEP);

		//フレームデータを用いて手の検出
		float dist_to_hand = detectHandInImage(frame, cascadefile, depth);
		//モーションの検出
		detectHandMoved(dists_vec, dist_to_hand);

		cv::imshow("window", frame);

		//qを押すと終了
		int key = cv::waitKey(1);
		if (key == 113) {
			break;
		}
	}

	cv::destroyAllWindows();

	return 0;
}

