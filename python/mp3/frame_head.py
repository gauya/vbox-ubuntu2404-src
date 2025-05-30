
def parse_mp3_header(header_bytes):
    header = int.from_bytes(header_bytes, byteorder='big')

    sync = (header >> 21) & 0x7FF
    if sync != 0x7FF:
        return None

    version_id = (header >> 19) & 0b11
    layer_desc = (header >> 17) & 0b11
    bitrate_index = (header >> 12) & 0b1111
    sampling_index = (header >> 10) & 0b11
    padding = (header >> 9) & 0b1

    versions = {
        0b00: "MPEG 2.5", 0b01: "reserved", 0b10: "MPEG 2", 0b11: "MPEG 1"
    }
    layers = {
        0b01: "Layer III", 0b10: "Layer II", 0b11: "Layer I"
    }

    bitrates = {
        "MPEG 1 Layer III": [None, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, None],
        "MPEG 2 Layer III": [None, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, None],
        "MPEG 2.5 Layer III": [None, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, None]
    }

    sampling_rates = {
        "MPEG 1": [44100, 48000, 32000],
        "MPEG 2": [22050, 24000, 16000],
        "MPEG 2.5": [11025, 12000, 8000]
    }

    version = versions.get(version_id, "unknown")
    layer = layers.get(layer_desc, "unknown")

    if version == "reserved" or layer == "unknown":
        return None

    bitrate_key = f"{version} {layer}"
    bitrate_list = bitrates.get(bitrate_key)
    if not bitrate_list or bitrate_index >= len(bitrate_list):
        return None

    bitrate_kbps = bitrate_list[bitrate_index]

    sr_list = sampling_rates.get(version)
    if not sr_list or sampling_index >= len(sr_list):
        return None

    sample_rate = sr_list[sampling_index]

    return {
        "version": version,
        "layer": layer,
        "bitrate_kbps": bitrate_kbps,
        "sampling_rate": sample_rate,
        "padding": padding
    }

"""
def parse_mp3_header(header_bytes):
    header = int.from_bytes(header_bytes, byteorder='big')
    
    sync = (header >> 21) & 0x7FF
    if sync != 0x7FF:
        return None  # Not a valid sync word
    
    version_id = (header >> 19) & 0b11
    layer_desc = (header >> 17) & 0b11
    bitrate_index = (header >> 12) & 0b1111
    sampling_index = (header >> 10) & 0b11
    padding = (header >> 9) & 0b1

    versions = {
        0b00: "MPEG 2.5", 0b01: "reserved", 0b10: "MPEG 2", 0b11: "MPEG 1"
    }
    layers = {
        0b01: "Layer III", 0b10: "Layer II", 0b11: "Layer I"
    }
    
    bitrates = {
        "MPEG 1 Layer III": [None, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, None],
        "MPEG 2 Layer III": [None, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, None]
    }

    sampling_rates = {
        "MPEG 1": [44100, 48000, 32000],
        "MPEG 2": [22050, 24000, 16000],
        "MPEG 2.5": [11025, 12000, 8000]
    }

    version = versions.get(version_id, "unknown")
    layer = layers.get(layer_desc, "unknown")

    if version == "reserved" or layer == "unknown":
        return None

    bitrate_key = f"{version} {layer}"
    bitrate_kbps = bitrates.get(bitrate_key, [None]*16)[bitrate_index]
    sample_rate = sampling_rates.get(version, [None]*3)[sampling_index]

    return {
        "version": version,
        "layer": layer,
        "bitrate_kbps": bitrate_kbps,
        "sampling_rate": sample_rate,
        "padding": padding
    }
"""

def scan_mp3_frames(filepath, max_frames=5):
    with open(filepath, 'rb') as f:
        data = f.read()

    found = 0
    i = 0
    while i < len(data) - 4:
        if data[i] == 0xFF and (data[i+1] & 0xE0) == 0xE0:
            header = parse_mp3_header(data[i:i+4])
            if header:
                print(f"Frame {found+1}: {header}")
                found += 1
                i += 100  # skip some bytes to avoid false positives
                if found >= max_frames:
                    break
                continue
        i += 1

# 사용 예시
fn="example.mp3"

import sys
if len(sys.argv) > 1:
  fn = sys.argv[1]

scan_mp3_frames(fn)

