// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SFTP_OPERATION_H_
#define ORBIT_SSH_QT_SFTP_OPERATION_H_

#include <OrbitSsh/SftpFile.h>
#include <OrbitSshQt/ScopedConnection.h>
#include <OrbitSshQt/Session.h>
#include <OrbitSshQt/SftpChannel.h>
#include <OrbitSshQt/StateMachineHelper.h>

#include <QFile>
#include <QObject>
#include <QPointer>
#include <filesystem>
#include <optional>

namespace OrbitSshQt {
namespace details {
enum class SftpOperationState {
  kInitial,
  kNoOperation,
  kStarted,
  kLocalFileOpened,
  kRemoteFileOpened,
  kRemoteFileWritten,
  kRemoteFileClosed,
  kShutdown,
  kDone,
  kError
};
}  // namespace details

/*
  SftpOperation represents a file operation in the SSH-SFTP subsystem.
  It needs an established SftpChannel for operation.

  Currently only file copies (local -> remote) are supported.
*/
class SftpOperation
    : public StateMachineHelper<SftpOperation, details::SftpOperationState> {
  Q_OBJECT
  friend StateMachineHelper;

 public:
  enum class FileMode : int {
    kUserWritable = 0644,
    kUserWritableAllExecutable = 0755
  };

  explicit SftpOperation(Session* session, SftpChannel* channel);

  void CopyFileToRemote(std::filesystem::path source,
                        std::filesystem::path destination,
                        FileMode destination_mode);

 signals:
  void started();
  void stopped();
  void aboutToShutdown();
  void errorOccurred(std::error_code);

 private:
  QPointer<Session> session_;

  std::optional<ScopedConnection> data_event_connection_;
  std::optional<ScopedConnection> about_to_shutdown_connection_;

  QPointer<SftpChannel> channel_;
  std::optional<OrbitSsh::SftpFile> sftp_file_;
  QFile local_file_;
  QByteArray write_buffer_;

  std::filesystem::path source_;
  std::filesystem::path destination_;
  FileMode destination_mode_;

  void HandleChannelShutdown();
  void HandleEagain();

  outcome::result<void> run();
  outcome::result<void> startup();
  outcome::result<void> shutdown();

  using StateMachineHelper::SetError;
  void SetError(std::error_code);
};

}  // namespace OrbitSshQt

#endif  // ORBIT_SSH_QT_SFTP_OPERATION_H_