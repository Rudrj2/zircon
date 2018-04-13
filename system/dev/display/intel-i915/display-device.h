// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ddk/protocol/display-controller.h>
#include <ddktl/device.h>
#include <hwreg/mmio.h>
#include <lib/edid/edid.h>
#include <region-alloc/region-alloc.h>
#include <lib/zx/vmo.h>

#include "gtt.h"
#include "power.h"
#include "registers-ddi.h"
#include "registers-pipe.h"
#include "registers-transcoder.h"

namespace i915 {

class Controller;

typedef struct default_display_info {
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t pixelsize;
} default_display_info_t;

class DisplayDevice {
public:
    DisplayDevice(Controller* device, uint64_t id,
                  registers::Ddi ddi, registers::Trans trans, registers::Pipe pipe);
    virtual ~DisplayDevice();

    void ApplyConfiguration(display_config_t* config);

    void Flush();
    bool Init();
    bool Resume();
    // Method to allow the display device to handle hotplug events. Returns
    // true if the device can handle the event without disconnecting. Otherwise
    // the device will be removed.
    virtual bool HandleHotplug(bool long_pulse) { return false; }

    uint64_t id() const { return id_; }
    const zx::vmo& framebuffer_vmo() const { return framebuffer_vmo_; }
    uint32_t framebuffer_size() const { return framebuffer_size_; }
    const default_display_info_t& info() const { return info_; }
    registers::Ddi ddi() const { return ddi_; }
    registers::Pipe pipe() const { return pipe_; }
    registers::Trans trans() const { return trans_; }
    Controller* controller() { return controller_; }
    const edid::Edid& edid() { return edid_; }

protected:
    // Queries the DisplayDevice to see if there is a supported display attached. If
    // there is, then returns true and populates |edid| and |info|.
    virtual bool QueryDevice(edid::Edid* edid, default_display_info_t* info) = 0;
    // Configures the hardware to display a framebuffer at the preferred resolution.
    virtual bool DefaultModeset() = 0;

    hwreg::RegisterIo* mmio_space() const;
    void ResetPipe();
    bool ResetTrans();
    bool ResetDdi();

private:
    // Borrowed reference to Controller instance
    Controller* controller_;

    uint64_t id_;
    registers::Ddi ddi_;
    registers::Trans trans_;
    registers::Pipe pipe_;

    PowerWellRef ddi_power_;
    PowerWellRef pipe_power_;

    uintptr_t framebuffer_;
    uint32_t framebuffer_size_;
    zx::vmo framebuffer_vmo_;
    fbl::unique_ptr<GttRegion> fb_gfx_addr_;

    bool inited_;
    default_display_info_t info_;
    uint32_t image_type_;
    edid::Edid edid_;
    bool is_enabled_;
};

} // namespace i915
