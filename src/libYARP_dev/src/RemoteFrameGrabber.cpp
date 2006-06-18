// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include <yarp/os/BufferedPort.h>
#include <yarp/os/Time.h>
#include <yarp/os/Network.h>
#include <yarp/os/Thread.h>
#include <yarp/os/Vocab.h>
#include <yarp/String.h>

#include <yarp/dev/FrameGrabberInterfaces.h>
#include <yarp/dev/PolyDriver.h>

using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;


namespace yarp{
    namespace dev {
      class TestFrameGrabber;
      class RemoteFrameGrabber;
      class ServerFrameGrabber;
    }
}

#define VOCAB_BRIGHTNESS VOCAB3('b','r','i')
#define VOCAB_SHUTTER VOCAB4('s','h','u','t')
#define VOCAB_GAIN VOCAB4('g','a','i','n')
#define VOCAB_SET VOCAB3('s','e','t')
#define VOCAB_GET VOCAB3('g','e','t')
#define VOCAB_IS VOCAB2('i','s')
#define VOCAB_WIDTH VOCAB1('w')
#define VOCAB_HEIGHT VOCAB1('h')


class yarp::dev::TestFrameGrabber : public DeviceDriver, 
            public IFrameGrabberImage, public IFrameGrabberControls {
private:
    int ct;
    int w, h;

public:
    TestFrameGrabber() {
        ct = 0;

        // just for nostalgia
        w = 128;
        h = 128;
    }


    virtual bool open() {
        return true;
    }

    virtual bool close() {
        return true;
    }

    virtual bool open(Searchable& prop) {
        BottleBit *val;
        if (prop.check("width",val)||prop.check("w",val)) {
            w = val->asInt();
        }
        if (prop.check("height",val)||prop.check("h",val)) {
            h = val->asInt();
        }
        return true;
    }

    virtual bool getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb>& image) {
        Time::delay(0.05);
        image.resize(w,h);
        image.zero();
        for (int i=0; i<image.width(); i++) {
            image.pixel(i,ct).r = 255;
        }
        ct++;
        if (ct>=image.height()) {
            ct = 0;
        }
        return true;
    }
    
    virtual int height() const {
        return h;
    }

    virtual int width() const {
        return w;
    }

    virtual bool setBrightness(double v) {
        return false;
    }

    virtual bool setShutter(double v) {
        return false;
    }

    virtual bool setGain(double v) {
        return false;
    }

    virtual double getBrightness() const {
        return 0;
    }

    virtual double getShutter() const {
        return 0;
    }

    virtual double getGain() const {
        return 0;
    }
};


class yarp::dev::ServerFrameGrabber : public DeviceDriver, public Thread,
            public PortReader,
            public IFrameGrabberImage, public IFrameGrabberControls
            // convenient to put these here just to make sure all
            // methods get implemented
{
private:
    Port p;
    PolyDriver poly;
    IFrameGrabberImage *fgImage;
    IFrameGrabberControls *fgCtrl;
    Property settings;
public:
    ServerFrameGrabber() {
        fgImage = NULL;
        fgCtrl = NULL;
    }
    
    virtual bool open() {
        return false;
    }
    
    virtual bool close() {
        return true;
    }
    
    virtual bool open(Searchable& prop) {
        p.setReader(*this);
        
        BottleBit *name;
        if (prop.check("subdevice",name)) {
            printf("Subdevice %s\n", name->toString().c_str());
            if (name->isString()) {
                // maybe user isn't doing nested configuration
                Property p;
                p.fromString(prop.toString());
                p.put("device",name->toString());
                poly.create(p);
            } else {
                poly.create(*name);
            }
            if (!poly.isValid()) {
                printf("cannot make <%s>\n", name->toString().c_str());
            }
        } else {
            printf("\"--subdevice <name>\" not set for server_framegrabber\n");
            return false;
        }
        if (prop.check("name",name)) {
            p.open(name->asString());
        } else {
            p.open("/grabber");
        }
        if (poly.isValid()) {
            poly.view(fgImage);
            poly.view(fgCtrl);
        }
        if (fgImage!=NULL) {
            start();
            return true;
        }
        printf("subdevice <%s> doesn't look like a framegrabber\n",
               name->toString().c_str());
        return false;
    }

    virtual void run() {
        printf("Server framegrabber starting\n");
        while (!isStopping()) {
            ImageOf<PixelRgb> img;
            getImage(img);
            printf("Fake framegrabber writing a %dx%d image...\n",
                   img.width(),img.height());
            p.write(img);
            Time::delay(0.05);
        }
        printf("Server framegrabber stopping\n");
    }

    virtual bool read(ConnectionReader& connection) {
        Bottle cmd, response;
        cmd.read(connection);
        printf("command received: %s\n", cmd.toString().c_str());
        int code = cmd.get(0).asVocab();
        switch (code) {
        case VOCAB_SET:
            printf("set command received\n");
            {
                bool ok = false;
                switch(cmd.get(1).asVocab()) {
                case VOCAB_BRIGHTNESS:
                    ok = setBrightness(cmd.get(2).asDouble());
                    break;
                case VOCAB_SHUTTER:
                    ok = setShutter(cmd.get(2).asDouble());
                    break;
                case VOCAB_GAIN:
                    ok = setGain(cmd.get(2).asDouble());
                    break;
                }
            }
            break;
        case VOCAB_GET:
            printf("get command received\n");
            {
                bool ok = false;
                response.addVocab(VOCAB_IS);
                response.addBit(cmd.get(1));
                switch(cmd.get(1).asVocab()) {
                case VOCAB_BRIGHTNESS:
                    ok = true;
                    response.addDouble(getBrightness());
                    break;
                case VOCAB_SHUTTER:
                    ok = true;
                    response.addDouble(getShutter());
                    break;
                case VOCAB_GAIN:
                    ok = true;
                    response.addDouble(getGain());
                    break;
                case VOCAB_WIDTH:
                    // normally, this would come from stream information
                    ok = true;
                    response.addInt(width());
                    break;
                case VOCAB_HEIGHT:
                    // normally, this would come from stream information
                    ok = true;
                    response.addInt(height());
                    break;
                }
                if (!ok) {
                    // leave answer blank
                }
            }
            break;
        }
        if (response.size()>=1) {
            ConnectionWriter *writer = connection.getWriter();
            if (writer!=NULL) {
                response.write(*writer);
                printf("response sent: %s\n", response.toString().c_str());
            }
        }
        return true;
    }

    virtual bool getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb>& image) {
        if (fgImage==NULL) { return false; }
        return fgImage->getImage(image);
    }
    
    virtual int height() const {
        if (fgImage==NULL) { return 0; }
        return fgImage->height();
    }

    virtual int width() const {
        if (fgImage==NULL) { return 0; }
        return fgImage->width();
    }

    virtual bool setBrightness(double v) {
        if (fgCtrl==NULL) { return false; }
        return fgCtrl->setBrightness(v);
    }

    virtual bool setShutter(double v) {
        if (fgCtrl==NULL) { return false; }
        return fgCtrl->setShutter(v);
    }

    virtual bool setGain(double v) {
        if (fgCtrl==NULL) { return false; }
        return fgCtrl->setGain(v);
    }

    virtual double getBrightness() const {
        if (fgCtrl==NULL) { return 0; }
        return fgCtrl->getBrightness();
    }

    virtual double getShutter() const {
        if (fgCtrl==NULL) { return 0; }
        return fgCtrl->getShutter();
    }

    virtual double getGain() const {
        if (fgCtrl==NULL) { return 0; }
        return fgCtrl->getGain();
    }
};


class yarp::dev::RemoteFrameGrabber : public IFrameGrabberImage, 
            public IFrameGrabberControls,
            public DeviceDriver {
private:
    Port port;
    PortReaderBuffer<ImageOf<PixelRgb> > reader;
    String remote;
    String local;
    int lastHeight;
    int lastWidth;
public:
    RemoteFrameGrabber() {
        lastHeight = 0;
        lastWidth = 0;
    }

    virtual ~RemoteFrameGrabber() {
    }

    virtual bool getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb>& image) {
        
        if (reader.read(true)!=NULL) {
            image = *(reader.lastRead());
            lastHeight = image.height();
            lastWidth = image.width();
            return true;
        }
        return false;
    }

    virtual int height() const {
        return lastHeight;
    }

    virtual int width() const {
        return lastWidth;
    }

    virtual bool open(){
        return true;
    }

    virtual bool open(Searchable& config){
        remote = config.find("remote").asString().c_str();
        local = config.find("local").asString().c_str();
        if (local!="") {
            port.open(local.c_str());
        }
        if (remote!="") {
            Network::connect(remote.c_str(),local.c_str());

            // reverse connection for RPC
            // could choose to do this only on need
            Network::connect(local.c_str(),remote.c_str());
        }
        reader.attach(port);
        return true;
    }

    virtual bool close() {
        return true;;
    }

    bool setCommand(int code, double v) {
        Bottle cmd;
        cmd.addVocab(VOCAB_SET);
        cmd.addVocab(code);
        cmd.addDouble(v);
        port.write(cmd);
        return true;
    }

    double getCommand(int code) const {
        Bottle cmd, response;
        cmd.addVocab(VOCAB_GET);
        cmd.addVocab(code);
        port.write(cmd,response);
        // response should be [cmd] [name] value
        return response.get(2).asDouble();
    }

    virtual bool setBrightness(double v) {
        return setCommand(VOCAB_BRIGHTNESS,v);
    }

    virtual bool setShutter(double v) {
        return setCommand(VOCAB_SHUTTER,v);
    }

    virtual bool setGain(double v) {
        return setCommand(VOCAB_GAIN,v);
    }

    virtual double getBrightness() const {
        return getCommand(VOCAB_BRIGHTNESS);
    }

    virtual double getShutter() const {
        return getCommand(VOCAB_SHUTTER);
    }

    virtual double getGain() const {
        return getCommand(VOCAB_GAIN);
    }
};

yarp::dev::DeviceDriver *createTestFrameGrabber() {
    return new yarp::dev::TestFrameGrabber();
}

yarp::dev::DeviceDriver *createRemoteFrameGrabber() {
    return new yarp::dev::RemoteFrameGrabber();
}

yarp::dev::DeviceDriver *createServerFrameGrabber() {
    return new yarp::dev::ServerFrameGrabber();
}
