#!/usr/bin/env ruby
require 'digest/sha1'
require 'base64'

GUID="258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
puts ARGV.first
puts ARGV.first.length
key = ARGV.first
raise ArgumentError, "Must provide a 24-byte key" if (key.nil? || key.length != 24)

digest = Digest::SHA1.digest(key+GUID)
puts Base64.encode64(digest)
