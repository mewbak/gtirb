#include "ImageByteMap.hpp"
#include "Serialization.hpp"
#include <proto/ImageByteMap.pb.h>

using namespace gtirb;

void ImageByteMap::setFileName(std::string X) { this->FileName = X; }

std::string ImageByteMap::getFileName() const { return this->FileName; }

void ImageByteMap::setBaseAddress(Addr X) { this->BaseAddress = X; }

Addr ImageByteMap::getBaseAddress() const { return this->BaseAddress; }

void ImageByteMap::setEntryPointAddress(Addr X) { this->EntryPointAddress = X; }

Addr ImageByteMap::getEntryPointAddress() const {
  return this->EntryPointAddress;
}

bool ImageByteMap::setAddrMinMax(std::pair<Addr, Addr> X) {
  using namespace std::rel_ops;

  if (X.first <= X.second) {
    this->EaMinMax = std::move(X);
    return true;
  }

  this->EaMinMax = std::pair<Addr, Addr>(Addr{}, Addr{});
  return false;
}

std::pair<Addr, Addr> ImageByteMap::getAddrMinMax() const {
  return this->EaMinMax;
}

void ImageByteMap::setRebaseDelta(int64_t X) { this->RebaseDelta = X; }

int64_t ImageByteMap::getRebaseDelta() const { return this->RebaseDelta; }

void ImageByteMap::setIsRelocated() { this->IsRelocated = true; }

bool ImageByteMap::getIsRelocated() const { return this->IsRelocated; }

boost::endian::order ImageByteMap::getByteOrder() const {
  return this->ByteOrder;
}

void ImageByteMap::setByteOrder(boost::endian::order Value) {
  this->ByteOrder = Value;
}

void ImageByteMap::setData(Addr Ea, gsl::span<const std::byte> Data) {
  using namespace std::rel_ops;

  if (Ea >= this->EaMinMax.first &&
      (Ea + Data.size_bytes() - 1) <= this->EaMinMax.second) {
    this->ByteMap.setData(Ea, Data);
  } else {
    throw std::out_of_range(
        "Attempt to set data at an Addr out of range of the min and max Addr.");
  }
}

void ImageByteMap::setData(Addr Ea, size_t Bytes, std::byte Value) {
  auto Span = gsl::make_span(&Value, 1);
  for (uint64_t I = 0; I < Bytes; I++) {
    this->ByteMap.setData(Ea + I, Span);
  }
}

std::vector<std::byte> ImageByteMap::getData(Addr X, size_t Bytes) const {
  using namespace std::rel_ops;

  if (X >= this->EaMinMax.first && (X + Bytes - 1) <= this->EaMinMax.second) {
    return this->ByteMap.getData(X, Bytes);
  }

  throw std::out_of_range(
      "Attempt to get data at an Addr out of range of the min and max Addr.");
}

void ImageByteMap::toProtobuf(MessageType* Message) const {
  nodeUUIDToBytes(this, *Message->mutable_uuid());
  this->ByteMap.toProtobuf(Message->mutable_byte_map());
  Message->set_file_name(this->FileName);
  Message->set_addr_min(static_cast<uint64_t>(this->EaMinMax.first));
  Message->set_addr_max(static_cast<uint64_t>(this->EaMinMax.second));
  Message->set_base_address(static_cast<uint64_t>(this->BaseAddress));
  Message->set_entry_point_address(
      static_cast<uint64_t>(this->EntryPointAddress));
  Message->set_rebase_delta(this->RebaseDelta);
  Message->set_is_relocated(this->IsRelocated);
}

void ImageByteMap::fromProtobuf(const MessageType& Message) {
  setNodeUUIDFromBytes(this, Message.uuid());
  this->ByteMap.fromProtobuf(Message.byte_map());
  this->FileName = Message.file_name();
  this->EaMinMax = {Addr(Message.addr_min()), Addr(Message.addr_max())};
  this->BaseAddress = Addr(Message.base_address());
  this->EntryPointAddress = Addr(Message.entry_point_address());
  this->RebaseDelta = Message.rebase_delta();
  this->IsRelocated = Message.is_relocated();
}
