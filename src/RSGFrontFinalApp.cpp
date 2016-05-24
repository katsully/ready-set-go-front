/*
*
* Copyright (c) 2015, Wieden+Kennedy
* Stephen Schieberl, Michael Latzoni
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or
* without modification, are permitted provided that the following
* conditions are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
*
* Neither the name of the Ban the Rewind nor the names of its
* contributors may be used to endorse or promote products
* derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

// Edited by Kat Sullivan

#include "cinder/app/App.h"
#include "cinder/params/Params.h"
#include "cinder/gl/gl.h" 
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"

#include "Kinect2.h"
#include "Osc.h"
#include "Outfit.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const std::string destinationHost = "127.0.0.1";
const uint16_t destinationPort = 8000;

class RSGFrontFinalApp : public ci::app::App
{
public:
	RSGFrontFinalApp();

	void						setup() override;
	void						draw() override;
	void						update() override;
	void						keyDown(KeyEvent event);
	vec3						makeBigger(vec3 hsvColor);
private:
	Kinect2::BodyFrame			mBodyFrame;
	ci::Channel8uRef			mChannelBodyIndex;
	ci::Channel16uRef			mChannelDepth;
	Kinect2::DeviceRef			mDevice;
	ci::Surface8uRef			mSurfaceColor;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;

	int updateCounter;

	// list of colors representing a person
	//vector<ColorA8u> shirtColors;
	vector < ColorA8u > bodyColors;
	vector < Outfit > outfits;

	//osc::SenderUdp mSender;
	osc::ReceiverUdp mReceiver;
};

//RGSFrontFinalApp::RGSFrontFinalApp() : App(), mReceiver(9000), mSender(8000, destinationHost, 8000)
RSGFrontFinalApp::RSGFrontFinalApp() : mReceiver(8000)
{
	//mSender.bind();
	console() << "START~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	mFrameRate = 0.0f;
	mFullScreen = false;

	mDevice = Kinect2::Device::create();
	mDevice->start();
	mDevice->connectBodyEventHandler([&](const Kinect2::BodyFrame frame)
	{
		mBodyFrame = frame;
	});
	mDevice->connectBodyIndexEventHandler([&](const Kinect2::BodyIndexFrame frame)
	{
		mChannelBodyIndex = frame.getChannel();
	});
	mDevice->connectDepthEventHandler([&](const Kinect2::DepthFrame frame)
	{
		mChannelDepth = frame.getChannel();
	});
	mDevice->connectColorEventHandler([&](const Kinect2::ColorFrame& frame)
	{
		mSurfaceColor = frame.getSurface();
	});

	mParams = params::InterfaceGl::create("Params", ivec2(200, 100));
	mParams->addParam("Frame rate", &mFrameRate, "", true);
	mParams->addParam("Full screen", &mFullScreen).key("f");
	mParams->addButton("Quit", [&]() { quit(); }, "key=q");

	bodyColors.push_back(Color(1, 0, 0));	// red
	bodyColors.push_back(Color(1, 1, 0));	// yellow
	bodyColors.push_back(Color(0, 1, 0));	// green
	bodyColors.push_back(Color(0, 0, 1));	// blue
	bodyColors.push_back(Color(0, 0, 0));	// black
	bodyColors.push_back(Color(1, 1, 1));	// white
	bodyColors.push_back(Color(1, 0, 1));	// purple
	bodyColors.push_back(Color(1, 0.5, 0)); // orange
}

void RSGFrontFinalApp::setup()
{
	updateCounter = 0;

	//mSender.bind();
	mReceiver.bind();
	mReceiver.listen();
	mReceiver.setListener("/kinect/blobs",
		[&](const osc::Message &msg) {
		console() << "MADE IITTT" << endl;
		console() << "ID: " << msg[0].int32() << endl;
		console() << "x coord: " << msg[1].flt() << endl;
		console() << "y coord: " << msg[2].flt() << endl;
	});
}

void RSGFrontFinalApp::draw()
{
	const gl::ScopedViewport scopedViewport(ivec2(0), getWindowSize());
	const gl::ScopedMatrices scopedMatrices;
	const gl::ScopedBlendAlpha scopedBlendAlpha;
	gl::setMatricesWindow(getWindowSize());
	gl::clear();
	gl::color(ColorAf::white());
	gl::disableDepthRead();
	gl::disableDepthWrite();

	if (mChannelDepth) {
		gl::enable(GL_TEXTURE_2D);
		const gl::TextureRef tex = gl::Texture::create(*Kinect2::channel16To8(mChannelDepth));
		gl::draw(tex, tex->getBounds(), Rectf(getWindowBounds()));
	}

	if (mChannelBodyIndex) {

		gl::enable(GL_TEXTURE_2D);

		auto drawHand = [&](const Kinect2::Body::Hand& hand, const ivec2& pos) -> void
		{
			switch (hand.getState()) {
			case HandState_Closed:
				gl::color(ColorAf(1.0f, 0.0f, 0.0f, 0.5f));
				break;
			case HandState_Lasso:
				gl::color(ColorAf(0.0f, 0.0f, 1.0f, 0.5f));
				break;
			case HandState_Open:
				gl::color(ColorAf(0.0f, 1.0f, 0.0f, 0.5f));
				break;
			default:
				gl::color(ColorAf(0.0f, 0.0f, 0.0f, 0.0f));
				break;
			}
			gl::drawSolidCircle(pos, 30.0f, 32);
		};

		const gl::ScopedModelMatrix scopedModelMatrix;
		gl::scale(vec2(getWindowSize()) / vec2(mChannelBodyIndex->getSize()));
		gl::disable(GL_TEXTURE_2D);
		for (const Kinect2::Body& body : mBodyFrame.getBodies()) {
			if (body.isTracked()) {
				bool newPerson = true; // if shirts and pants match a tracked outfit
				int idx = 0;	// this will correspond to the index of bodyColors
				ColorA8u shirtColor;
				ColorA8u pantColor;
				vec3 shirtHSV;
				vec3 pantHSV;
				for (const auto& joint : body.getJointMap()) {
					if (joint.second.getTrackingState() == TrackingState::TrackingState_Tracked) {
						// get the color from the performer's right shoulder
						if (joint.first == JointType_ShoulderRight) {
							// get coordinate of shoulder from RGB camera
							const vec2 rShoulderPos = mDevice->mapCameraToColor(joint.second.getPosition());
							// get color of the shirt
							shirtColor = mSurfaceColor->getPixel(rShoulderPos);
							gl::color(shirtColor);
							gl::drawSolidCircle(vec2(30.0, 30.0), 30.0);
							shirtHSV = shirtColor.get(CM_HSV);
							shirtHSV = makeBigger(shirtHSV);
							console() << "SHIRT" << endl;
							console() << "hue " << shirtHSV.x << endl;
							console() << "saturation " << shirtHSV.y << endl;
							console() << "value " << shirtHSV.z << endl;
						}
						// second color tracker to look at pant color
						else if (joint.first == JointType_KneeRight) {
							// get coordinate of knee from RGB camera
							const vec2 rKneePos = mDevice->mapCameraToColor(joint.second.getPosition());
							// get color of the pants
							pantColor = mSurfaceColor->getPixel(rKneePos);
							/*gl::color(pantColor);
							gl::drawSolidCircle(vec2(10.0, 10.0), 20.0);*/
							pantHSV = pantColor.get(CM_HSV);
							pantHSV = makeBigger(pantHSV);
							console() << "PANTS" << endl;
							console() << "hue " << pantHSV.x << endl;
							console() << "saturation " << pantHSV.y << endl;
							console() << "value " << pantHSV.z << endl;
							// match body to previously tracked body
							float closestMatch = 10000.0f;
							Outfit matchingOutfit;
							int closestIdx;
							float match;
							bool matchBool;
							for (Outfit &outfit : outfits) {
								match = 100000.0f;
								matchBool = false;
								ColorA8u c = outfit.shirtColor;
								vec3 cHSV = c.get(CM_HSV);
								cHSV = makeBigger(cHSV);
								// match shirts
								/*console() << "ABS shirts" << endl;
								console() << abs(shirtHSV.x - cHSV.x) << endl;
								console() << abs(shirtHSV.y - cHSV.y) << endl;
								console() << abs(shirtHSV.z - cHSV.z) << endl;*/
								if (abs(shirtHSV.x - cHSV.x) < 15 && abs(shirtHSV.y - cHSV.y) < 120 && abs(shirtHSV.z - cHSV.z) < 25) {
									ColorA8u p = outfit.pantColor;
									vec3 pHSV = p.get(CM_HSV);
									pHSV = makeBigger(pHSV);
									match = abs(shirtHSV.x - cHSV.x) + (abs(shirtHSV.y - cHSV.y)/10) + abs(shirtHSV.z - cHSV.z);
									console() << "shirt matched" << endl;
									/*console() << "ABS pants" << endl;
									console() << abs(pantHSV.x - pHSV.x) << endl;
									console() << abs(pantHSV.y - pHSV.y) << endl;
									console() << abs(pantHSV.z - pHSV.z) << endl;*/
									// match pants
									if (abs(pantHSV.x - pHSV.x) < 25 && abs(pantHSV.y - pHSV.y) < 120 && abs(pantHSV.z - pHSV.z) < 120) {
										// definetly an exisiting identified body
										newPerson = false;
										matchBool = true;
										//outfit.update(shirtColor, pantColor);
										console() << "~~~~~~tracked person~~~~~~~" << endl;
										//match += abs(pantHSV.x - pHSV.x) + abs(pantHSV.y - pHSV.y) + abs(pantHSV.z - pHSV.z);
										console() << "match" << idx << ": " << match << endl;
										//break;
									}
								}
								if (matchBool && (closestMatch > match)) {
									console() << "in the if statement" << endl;
									closestMatch = match;
									matchingOutfit = outfit;
									closestIdx = idx;
								}
								idx++;
							}
							// add new person to list of identified bodies
							if (newPerson) {
								console() << "NEW PERSONNN!!!!!!!!" << endl;
								outfits.push_back(Outfit(shirtColor, pantColor));
								idx = outfits.size() - 1;
							}
							else {
								//matchingOutfit.update(shirtColor, pantColor);
								idx = closestIdx;
							}
							if (idx >= 8) {
								idx = 5;
							}
							console() << "OUTFITS SIZE: ";
							console() << outfits.size() << endl;
							console() << "INDEX COUNT: ";
							console() << idx << endl;
							console() << "--------------------------" << endl;
							gl::color(bodyColors[idx]);
						}
					}
				}
				// draw joints
				gl::color(bodyColors[idx]);
				for (const auto& joint : body.getJointMap()) {
					if (joint.second.getTrackingState() == TrackingState::TrackingState_Tracked) {
						const vec2 pos(mDevice->mapCameraToDepth(joint.second.getPosition()));
						gl::drawSolidCircle(pos, 5.0f, 32);
						const vec2 parent(mDevice->mapCameraToDepth(
							body.getJointMap().at(joint.second.getParentJoint()).getPosition()
							));
						gl::drawLine(pos, parent);
					}
				}
				drawHand(body.getHandLeft(), mDevice->mapCameraToDepth(body.getJointMap().at(JointType_HandLeft).getPosition()));
				drawHand(body.getHandRight(), mDevice->mapCameraToDepth(body.getJointMap().at(JointType_HandRight).getPosition()));
			}
		}
	}

	mParams->draw();
}

static bool pred(Outfit &o) {
	return o.appearances < 10;
}

void RSGFrontFinalApp::update()
{
	mFrameRate = getAverageFps();

	if (mFullScreen != isFullScreen()) {
		setFullScreen(mFullScreen);
		mFullScreen = isFullScreen();
	}


	//if (updateCounter % 150 == 0) {
	//	// remove all outfit objects that have not been tracked more than 20 times frames
	//	outfits.erase(std::remove_if(outfits.begin(), outfits.end(), pred), outfits.end());
	//}
	//updateCounter++;
}

void RSGFrontFinalApp::keyDown(KeyEvent event) {
	if ('a' == event.getChar()) {
		console() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	}
}

vec3 RSGFrontFinalApp::makeBigger(vec3 hsvColor)
{
	vec3 newValues;
	newValues.x = hsvColor.x * 179;
	newValues.y = hsvColor.y * 255;
	newValues.z = hsvColor.z;
	return newValues;
}

CINDER_APP(RSGFrontFinalApp, RendererGl, [](App::Settings* settings)
{
	settings->prepareWindow(Window::Format().size(1024, 768).title("Body Tracking App"));
	settings->disableFrameRate();
})