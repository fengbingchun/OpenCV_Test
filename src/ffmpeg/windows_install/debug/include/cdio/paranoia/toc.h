/*
  Copyright (C) 2019
    Rocky Bernstein <rocky@gnu.org>
*/

/** \file toc.h
 *
 *  \brief Information from a CDROM TOC relevant to CD-DA disks
 ******************************************************************/

#ifndef CDIO__PARANOIA__TOC_H_
#define CDIO__PARANOIA__TOC_H_

#include <cdio/cdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /** cdrom_paranoia is an opaque structure which is used in all of the
      library operations.
  */
  typedef struct cdrom_paranoia_s cdrom_paranoia_t;
  typedef struct cdrom_drive_s   cdrom_drive_t;

  /*! Return the track containing the given LSN. If the LSN is before
    the first track (in the pregap), 0 is returned. If there was an
    error or the LSN after the LEADOUT (beyond the end of the CD), then
    CDIO_INVALID_TRACK is returned.
  */
  extern int     cdio_cddap_sector_gettrack(cdrom_drive_t *d, lsn_t lsn);

  /*! Return the number of channels in track: 2 or 4; -2 if not
    implemented or -1 for error.
    Not meaningful if track is not an audio track.
  */
  extern int     cdio_cddap_track_channels(cdrom_drive_t *d, track_t i_track);

  /*! Return 1 is track is an audio track, 0 otherwise. */
  extern int     cdio_cddap_track_audiop(cdrom_drive_t *d, track_t i_track);

  /*! Return 1 is track has copy permit set, 0 otherwise. */
  extern int     cdio_cddap_track_copyp(cdrom_drive_t *d, track_t i_track);

  /*! Return 1 is audio track has linear preemphasis set, 0 otherwise.
    Only makes sense for audio tracks.
  */
  extern int     cdio_cddap_track_preemp(cdrom_drive_t *d, track_t i_track);

  /*! Get first lsn of the first audio track. -1 is returned on error. */
  extern lsn_t   cdio_cddap_disc_firstsector(cdrom_drive_t *d);

  /*! Get last lsn of the last audio track. The last lsn is generally one
    less than the start of the next track after the audio track. -1 is
    returned on error. */
  extern lsn_t   cdio_cddap_disc_lastsector(cdrom_drive_t *d);

#ifdef __cplusplus
}
#endif

#endif /*CDIO__PARANOIA__TOC_H_*/
