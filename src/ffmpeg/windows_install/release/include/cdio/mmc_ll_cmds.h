/*
    Copyright (C) 2018 Thomas Schmitt
    Copyright (C) 2010, 2012, 2016, 2019 Rocky Bernstein <rocky@gnu.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
   \file mmc_ll_cmds.h

   \brief Wrappers for specific Multimedia Command (MMC) commands e.g., READ
   DISC, START/STOP UNIT.

   The documents we make use of are described in several
   specifications made by the SCSI committee T10
   http://www.t10.org. In particular, SCSI Primary Commands (SPC),
   SCSI Block Commands (SBC), and Multi-Media Commands (MMC). These
   documents generally have a numeric level number appended. For
   example SPC-3 refers to ``SCSI Primary Commands - 3'.

   In year 2010 the current versions were SPC-3, SBC-2, MMC-5.

*/

#ifndef CDIO_MMC_LL_CMDS_H_
#define CDIO_MMC_LL_CMDS_H_

#include <cdio/mmc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /**
     Get drive capabilities via SCSI-MMC \p GET \p CONFIGURATION
     @param p_cdio the CD object to be acted upon.

     @param p_buf pointer to location to store mode sense information

     @param i_size number of bytes allocated to p_buf

     @param i_return_type value in range 0..2 giving what kind of configuration
     to return:

     - 0 &mdash; Full Header and Full Descriptors;
     - 1 &mdash; Feature Headers and those with their Current Bit.
     - 2 &mdash; One Feature header and zero or one Feature Descriptors.

     @param i_starting_feature_number feature number from which to start
     getting information.

     @param i_timeout_ms value in milliseconds to use on timeout. Setting
     to 0 uses the default time-out value stored in
     mmc_timeout_ms.

     @return \p DRIVER_OP_SUCCESS (0) if we got the status.
     return codes are the same as \p driver_return_code_t
  */
    driver_return_code_t
    mmc_get_configuration(const CdIo_t *p_cdio, void *p_buf,
                          unsigned int i_size,
                          unsigned int i_return_type,
                          unsigned int i_starting_feature_number,
                          unsigned int i_timeout_ms);

  /**
    Return results of media event status via SCSI-MMC \p GET \p EVENT \p STATUS

    @param p_cdio the CD object to be acted upon.

    @param out_buf media status code from operation

    @return \p DRIVER_OP_SUCCESS (0) if we got the status. Return codes
    are the same as \p driver_return_code_t
   */
  driver_return_code_t mmc_get_event_status(const CdIo_t *p_cdio,
                                            uint8_t out_buf[2]);


   /**
      Run a SCSI-MMC \p MODE \p SELECT (10-byte) command
      and put the results in \p p_buf.

      @param p_cdio the CD object to be acted upon.

      @param p_buf pointer to location to store mode sense information

      @param i_size number of bytes allocated to p_buf

      @param page which "page" of the mode sense command we are interested in

      @param i_timeout_ms value in milliseconds to use on timeout. Setting
             to 0 uses the default time-out value stored in
             mmc_timeout_ms.

      @return \p DRIVER_OP_SUCCESS if we ran the command ok.
   */
  driver_return_code_t mmc_mode_select_10(CdIo_t *p_cdio, /*out*/ void *p_buf,
                                          unsigned int i_size, int page,
                                          unsigned int i_timeout_ms);
  /**
     Run a SCSI-MMC \p MODE \p SENSE command (10-byte version)
     and put the results in \p p_buf.
     @param p_cdio the CD object to be acted upon.
     @param p_buf pointer to location to store mode sense information

     @param i_size number of bytes allocated to p_buf

     @param i_page_code which "page" of the mode sense command we are
     interested in

     @return \p DRIVER_OP_SUCCESS if we ran the command ok.
  */
  driver_return_code_t mmc_mode_sense_10( CdIo_t *p_cdio, /*out*/ void *p_buf,
                                          unsigned int i_size,
                                          unsigned int i_page_code);

  /**
      Run a SCSI-MMC \p MODE \p SENSE command (6-byte version)
      and put the results in \p p_buf.
      @param p_cdio the CD object to be acted upon.
      @param p_buf pointer to location to store mode sense information
      @param i_size number of bytes allocated to p_buf
      @param page which "page" of the mode sense command we are interested in
      @return \p DRIVER_OP_SUCCESS if we ran the command ok.
  */
  driver_return_code_t  mmc_mode_sense_6( CdIo_t *p_cdio, /*out*/ void *p_buf,
                                          unsigned int i_size, int page);

  /**
     Request preventing/allowing medium removal on a drive via
     SCSI-MMC \p PREVENT/ALLOW \p MEDIUM \p REMOVAL.

     @param p_cdio the CD object to be acted upon.
     @param b_persistent make b_prevent state persistent
     @param b_prevent true of drive locked and false if unlocked

     @param i_timeout_ms value in milliseconds to use on timeout. Setting
            to 0 uses the default time-out value stored in
            mmc_timeout_ms.

     @return \p DRIVER_OP_SUCCESS (0) if we got the status.
     return codes are the same as \p driver_return_code_t
  */
    driver_return_code_t
    mmc_prevent_allow_medium_removal(const CdIo_t *p_cdio,
                                     bool b_persistent, bool b_prevent,
                                     unsigned int i_timeout_ms);

  /**
      Issue a MMC READ_CD command.

      @param p_cdio object to read from

      @param p_buf  Place to store data. The caller should ensure that
      \p p_buf can hold at least \p i_blocksize * \p i_blocks  bytes.

      @param i_lsn  sector to read

      @param expected_sector_type restricts reading to a specific CD
      sector type.  Only 3 bits with values 1-5 are used:
      - 0 &mdash; all sector types
      - 1 &mdash; CD-DA sectors only
      - 2 &mdash; Mode 1 sectors only
      - 3 &mdash; Mode 2 formless sectors only. Note in contrast to all other
      values an MMC CD-ROM is not required to support this mode.
      - 4 &mdash; Mode 2 Form 1 sectors only
      - 5 &mdash; Mode 2 Form 2 sectors only

    @param b_digital_audio_play Control error concealment when the
    data being read is CD-DA.  If the data being read is not CD-DA,
    this parameter is ignored.  If the data being read is CD-DA and
    DAP is false zero, then the user data returned should not be
    modified by flaw obscuring mechanisms such as audio data mute and
    interpolate.  If the data being read is CD-DA and DAP is true,
    then the user data returned should be modified by flaw obscuring
    mechanisms such as audio data mute and interpolate.

    b_sync_header return the sync header (which will probably have
    the same value as \p CDIO_SECTOR_SYNC_HEADER of size
    \p CDIO_CD_SYNC_SIZE).

    @param header_codes Header Codes refer to the sector header and
    the sub-header that is present in mode 2 formed sectors:

    - 0 &mdash; No header information is returned.
    - 1 &mdash;  The 4-byte sector header of data sectors is be returned,
    - 2 &mdash; The 8-byte sector sub-header of mode 2 formed sectors is
    returned.
    - 3 &mdash Both sector header and sub-header (12 bytes) is returned.

    The Header preceeds the rest of the bytes (e.g. user-data bytes)
    that might get returned.

    @param b_user_data  Return user data if true.

    For CD-DA, the User Data is \p CDIO_CD_FRAMESIZE_RAW bytes.

    For Mode 1, The User Data is \p ISO_BLOCKSIZE bytes beginning at
    offset \p CDIO_CD_HEADER_SIZE+CDIO_CD_SUBHEADER_SIZE.

    For Mode 2 formless, The User Data is M2RAW_SECTOR_SIZE bytes
    beginning at offset \p CDIO_CD_HEADER_SIZE+CDIO_CD_SUBHEADER_SIZE.

    For data Mode 2, form 1, User Data is ISO_BLOCKSIZE bytes beginning at
    offset \p CDIO_CD_XA_SYNC_HEADER.

    For data Mode 2, form 2, User Data is 2 groups of 324 bytes beginning at
    offset \p CDIO_CD_XA_SYNC_HEADER.

    @param b_sync

    @param b_edc_ecc true if we return EDC/ECC error detection/correction bits.

    The presence and size of EDC redundancy or ECC parity is defined
    according to sector type:

    CD-DA sectors have neither EDC redundancy nor ECC parity.

    Data Mode 1 sectors have 288 bytes of EDC redundancy, Pad, and
    ECC parity beginning at offset 2064.

    Data Mode 2 formless sectors have neither EDC redundancy nor ECC
    parity

    Data Mode 2 form 1 sectors have 280 bytes of EDC redundancy and
    ECC parity beginning at offset 2072

    Data Mode 2 form 2 sectors optionally have 4 bytes of EDC
    redundancy beginning at offset 2348.


    @param c2_error_information If true associate a bit with each
    sector for C2 error The resulting bit field is ordered exactly as
    the main channel bytes.  Each 8-bit boundary defines a byte of
    flag bits.

    @param subchannel_selection subchannel-selection bits

    - 0 &mdash; No Sub-channel data shall be returned.  (0 bytes)
    - 1 &mdash;  RAW P-W Sub-channel data shall be returned.  (96 byte)
    - 2 &mdash;  Formatted Q sub-channel data shall be transferred (16 bytes)
    - 3 &mdash; Reserved
    - 4 &mdash;  Corrected and de-interleaved R-W sub-channel (96 bytes)
    - 5-7 &mdash; Reserved

    @param i_blocksize size of the a block expected to be returned

    @param i_blocks number of blocks expected to be returned.

    @return \p DRIVER_OP_SUCCESS if we ran the command ok.
  */
  driver_return_code_t
  mmc_read_cd(const CdIo_t *p_cdio, void *p_buf, lsn_t i_lsn,
              int expected_sector_type, bool b_digital_audio_play,
              bool b_sync, uint8_t header_codes, bool b_user_data,
              bool b_edc_ecc, uint8_t c2_error_information,
              uint8_t subchannel_selection, uint16_t i_blocksize,
              uint32_t i_blocks);

  /**
     Request information about et drive capabilities via SCSI-MMC READ
     DISC INFORMATION

     @param p_cdio the CD object to be acted upon.

     @param p_buf pointer to location to store mode sense information

     @param i_size number of bytes allocated to p_buf

     @param data_type kind of information to retrieve.

     @param i_timeout_ms value in milliseconds to use on timeout. Setting
     to 0 uses the default time-out value stored in
     mmc_timeout_ms.

     @return DRIVER_OP_SUCCESS (0) if we got the status.
  */
 driver_return_code_t
 mmc_read_disc_information(const CdIo_t *p_cdio, /*out*/ void *p_buf,
                           unsigned int i_size,
                           cdio_mmc_read_disc_info_datatype_t data_type,
                           unsigned int i_timeout_ms);

  /**
    Set the drive speed in K bytes per second using SCSI-MMC SET SPEED.


    @param p_cdio        CD structure set by cdio_open().
    @param i_Kbs_speed   speed in K bytes per second. Note this is
                         not in standard CD-ROM speed units, e.g.
                         1x, 4x, 16x as it is in cdio_set_speed.
                         To convert CD-ROM speed units to Kbs,
                         multiply the number by 176 (for raw data)
                         and by 150 (for filesystem data).
                         Also note that ATAPI specs say that a value
                         less than 176 will result in an error.
                         On many CD-ROM drives,
                         specifying a value too large will result in using
                         the fastest speed.

    @param i_timeout_ms value in milliseconds to use on timeout. Setting
           to 0 uses the default time-out value stored in
           mmc_timeout_ms.

    @return the drive speed if greater than 0. -1 if we had an error. is -2
    returned if this is not implemented for the current driver.

    @see cdio_set_speed and mmc_set_drive_speed
  */
  driver_return_code_t mmc_set_speed( const CdIo_t *p_cdio,
                                      int i_Kbs_speed,
                                      unsigned int i_timeout_ms);

  /**
    Load or Unload media using a MMC START STOP UNIT command.

    @param p_cdio  the CD object to be acted upon.

    @param b_eject eject if true and close tray if false

    @param b_immediate wait or don't wait for operation to complete

    @param power_condition Set CD-ROM to idle/standby/sleep. If nonzero,
           eject/load is ignored, so set to 0 if you want to eject or load.

    @param i_timeout_ms value in milliseconds to use on timeout. Setting
           to 0 uses the default time-out value stored in
           mmc_timeout_ms.

    @return \p DRIVER_OP_SUCCESS if we ran the command ok.

    @see mmc_eject_media or mmc_close_tray
  */
  driver_return_code_t mmc_start_stop_unit(const CdIo_t *p_cdio, bool b_eject,
                                           bool b_immediate,
                                           uint8_t power_condition,
                                           unsigned int i_timeout_ms);

    /**
     Check if drive is ready using SCSI-MMC \p TEST \p UNIT \p READY command.

     @param p_cdio  the CD object to be acted upon.

     @param i_timeout_ms value in milliseconds to use on timeout. Setting
            to 0 uses the default time-out value stored in
            \p mmc_timeout_ms.

     @return \p DRIVER_OP_SUCCESS if we ran the command ok.
  */
  driver_return_code_t mmc_test_unit_ready(const CdIo_t *p_cdio,
                                           unsigned int i_timeout_ms);

/**
   Issue a \p READ \p SUB-CHANNEL command to read current position, \p ISRC or
   \p MCN from subchannel Q.
   Note: \p READ \p SUB-CHANNEL is deprecated as of MMC-5
   but the alternative requires manual parsing of the subchannel.

   @param p_cdio  the CD object to be acted upon.
   @param i_track track number (only for ISRC)
   @param sub_chan_param 1 for CD current position, 2 for \p MCN, 3 for \p ISRC
   @param i_length pointer to number of bytes to request.
                   Will be overwritten by the number of bytes available.
   @param p_buf  pointer to the location for the returned data
   @param i_timeout_ms numober of milliseconds to wait for command completion

   @return \p DRIVER_OP_SUCCESS on success
 */
driver_return_code_t
mmc_read_subchannel ( const CdIo_t *p_cdio,
                             track_t i_track,
                             unsigned char sub_chan_param,
                             unsigned int *i_length,
                             char *p_buf,
                             unsigned int i_timeout_ms
                            );


/**
   Issue a READ TOC/PMA/ATIP command to read the CD-TEXT from R-W
   sub-channel.

   On a successful return (when \p DRIVER_OP_SUCCESS is returned), \p
   pbuf contains the raw SCSI/MMC reply as retrieved by
   mmc_read_toc_cdtext().

   The first two bytes of the header, a Big-Endian number, specifies
   the number of following bytes. The count also includes the next two
   header bytes which should be 0. See also Table 495, of the MMC-5
   specification.

   Here is code that can be used to get the number of text packs:
   @code
     #include <cdio/mmc.h>
     CDIO_MMC_GET_LEN16(p_buf) - 2
   @endcode
   and start of the text packs is at:
   @code
     p_buf + 4
   @endcode

   The number of bytes returned is limited by the value in parameter
   \p *i_length. which should tell the byte capacity of \p p_buf.  The
   maximum size according to specification is 4 + 8 * 256 * 18 =
   36,864 bytes.

   Instead of retrieving everything in one go by allocating the
   maximum possible value in the passed-in \p *p_buf, another approach
   is to first get the header bytes in a small \p p_buf, then
   re-allocate the buffer using the length given by \p
   CDIO_MMC_GET_LEN16(p_buf) + 2 (bytes), and then to call
   mmc_read_toc_cdtext() again with \p *i_length set to this size.

   @param p_cdio  the CD object to be acted upon.
   @param i_length pointer to number of bytes to request.
                   Will be overwritten by the number of bytes available.
   @param p_buf  pointer to the location for the returned data
   @param i_timeout_ms value in milliseconds to use on timeout. Setting
          to 0 uses the default time-out value stored in
          \p mmc_timeout_ms.

   @return \p DRIVER_OP_SUCCESS on success
 */
driver_return_code_t
mmc_read_toc_cdtext ( const CdIo_t *p_cdio, unsigned int *i_length,
                      unsigned char *p_buf, unsigned int i_timeout_ms );


#ifndef DO_NOT_WANT_OLD_MMC_COMPATIBILITY
#define mmc_start_stop_media(c, e, i, p, t) \
    mmc_start_stop_unit(c, e, i, p, t, 0)
#endif /*DO_NOT_WANT_PARANOIA_COMPATIBILITY*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDIO_MMC_HL_CMDS_H_ */

/*
 * Local variables:
 *  c-file-style: "gnu"
 *  tab-width: 8
 *  indent-tabs-mode: nil
 * End:
 */
